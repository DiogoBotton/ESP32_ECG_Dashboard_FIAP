#include <time.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "secrets.h"

// Definição das constantes
const char* wifi_ssid = WIFI_SSID;
const char* wifi_pwd = WIFI_PWD;

const char* mqtt_ip = MQTT_IP;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pwd = MQTT_PWD;
const char* mqtt_queue = MQTT_QUEUE;

// Definição dos pinos do AD8232
#define ECG_PIN 36       // Pino de saída analógica do AD8232 (OUTPUT)
#define LO_PLUS_PIN 25   // Lead-off positivo (LO+)
#define LO_MINUS_PIN 26  // Lead-off negativo (LO-)

// Definição do pino e tipo do DHT
#define DHTPIN 19
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Declarando variaveis de Wifi e MQTT
//WiFiClientSecure espClient; // Conexão para o HiveMQ (necessário para conexão TLS via ESP32)
WiFiClient espClient; // Conexão para o RabbitMQ
PubSubClient client(espClient);

// Funções com paramêtros precisam ser declaradas antes de serem utilizadas
bool MountSPIFFS(int maxRetries = 5) {
  int attempts = 0;
  while (!SPIFFS.begin(true)) {
    Serial.println("Falha ao montar SPIFFS, tentando novamente...");
    attempts++;
    delay(500);
    if (attempts >= maxRetries) {
      Serial.println("Não foi possível montar o SPIFFS após várias tentativas.");
      return false;
    }
  }

  Serial.println("SPIFFS montado com sucesso!");
  return true;
}

void setup() {
  Serial.begin(115200);

  // Aguarda SPIFFS ser montado
  if (!MountSPIFFS()) {
    Serial.println("Erro crítico ao montar SPIFFS. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  pinMode(LO_PLUS_PIN, INPUT);
  pinMode(LO_MINUS_PIN, INPUT);
  pinMode(ECG_PIN, INPUT);

  //espClient.setInsecure(); // Necessário desabilitar o certificado para enviar os dados ao HiveMQ

  dht.begin();
  delay(2000);

  // Conecta ao Wifi e RabbitMQ
  Reconnect();

  // Sincroniza o fuso horário (NTP - Network Time Protocol)
  SyncTime();
}

void SyncTime(){
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.google.com");
  Serial.println("Sincronizando com NTP...");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao sincronizar com NTP. Usando millis() como fallback.");
  } else {
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.println("Data/Hora sincronizada: " + String(timeStr));
  }
}

void Reconnect(){
  ConnectWifi();
  ConnectRabbitMQ();
}

void ConnectWifi(){
  int errorCount = 0;

  WiFi.begin(wifi_ssid, wifi_pwd);
  while (WiFi.status() != WL_CONNECTED && errorCount < 3) {
    errorCount++;
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Conectado ao Wifi.");
}

void ConnectRabbitMQ(){
  int errorCount = 0;

  client.setServer(mqtt_ip, mqtt_port);

  String client_id = "esp32-client-" + String(WiFi.macAddress());

  // Conectar ao RabbitMQ
  while (!client.connected() && errorCount < 2) {
    Serial.println("Conectando ao RabbitMQ...");

    if (client.connect(client_id.c_str(), mqtt_user, mqtt_pwd)) {
      Serial.println("Conectado ao RabbitMQ via MQTT");
      client.subscribe(mqtt_queue);
      client.publish(mqtt_queue, "Estou conectado :)");
    } else {
      Serial.print("Falha na conexão, código: ");
      Serial.println(client.state());
      errorCount++;
      delay(1000);
    }
  }
}

// Função para salvar os dados no SPIFFS
void SaveToSPIFFS(const char* data) {
  // Limite de armazenamento
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;

  if (freeBytes < 2000) { // ~2KB livres
    Serial.println("Memória quase cheia, não salvando mais dados.");
    return;
  }

  String filename = "/data_" + String(millis()) + ".json";
  File file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("Erro ao abrir arquivo para escrita.");
    return;
  }

  file.println(data);
  file.close();
  Serial.println("Dados salvos no SPIFFS: " + filename);
  LogFreeBytes();
}

void LogFreeBytes(){
  Serial.printf("Espaço usado: %d / %d bytes\n", SPIFFS.usedBytes(), SPIFFS.totalBytes());
}

// Função para enviar os dados armazenados no SPIFFS
void SendSavedData() {
  if (!CheckConnection()) {
    Serial.println("Sem internet ou MQTT desconectado. Aguardando para reenviar...");
    return;
  }

  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("Erro ao abrir diretório raiz.");
    return;
  }

  // Coleta nomes dos arquivos em um vetor
  std::vector<String> files;
  File file = root.openNextFile();

  while (file) {
    String filename = file.name();
    if (!filename.startsWith("/")) {
      filename = "/" + filename; // Garante barra inicial
    }
    if (filename.startsWith("/data_")) { // Apenas arquivos relevantes (apenas os jsons criados)
      files.push_back(filename);
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();

  // Ordena os arquivos em ordem alfabética (baseado no millis)
  std::sort(files.begin(), files.end());

  // Processa os arquivos ordenados
  for (const auto& filename : files) {
    File f = SPIFFS.open(filename, FILE_READ);
    if (!f) {
      Serial.println("Erro ao abrir " + filename + " para leitura. Pulando...");
      continue;
    }

    String content = f.readString();
    f.close();

    Serial.println("Reenviando arquivo: " + filename);
    if (client.publish(mqtt_queue, content.c_str())) {
      Serial.println("Enviado com sucesso, tentando apagar arquivo...");
      if (!SPIFFS.remove(filename)) {
        Serial.println("Não foi possível remover o arquivo: " + filename);
      } else {
        Serial.println("Arquivo " + filename + " removido com sucesso.");
      }
    } else {
      Serial.println("Falha ao enviar, mantendo arquivo...");
      break; // Sai do loop para tentar novamente depois
    }

    delay(800);
  }
}

String GetDateTime(){
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return timeStr;
  } else {
    // Usa micros() se NTP não estiver disponível
    return String("millis_") + String(micros());
  }
}

bool CheckConnection(){
  return WiFi.status() == WL_CONNECTED && client.connected();
}

float calculateBPM(int ecgValue) {
  const int threshold = 2000;  // limite de detecção
  const unsigned long debounceTime = 250;  // tempo mínimo entre batimentos (ms) (~240 BPM)
  
  static unsigned long lastBeatTime = 0;
  static bool aboveThreshold = false;
  static float bpm = 0;

  unsigned long now = millis();

  // --- Detecta subida acima do limiar ---
  if (ecgValue > threshold && !aboveThreshold) {
    aboveThreshold = true;
    if (lastBeatTime > 0) {
      unsigned long delta = now - lastBeatTime;
      if (delta > debounceTime) {
        bpm = 60000.0 / delta;  // converte intervalo (ms) em batimentos/minuto
      }
    }
    lastBeatTime = now;
  }

  if (ecgValue < threshold) {
    aboveThreshold = false;
  }

  return bpm;
}

void loop() {
  // Caso estiver conectado ao MQTT (RabbitMQ) mantém a conexão estável
  if (client.connected()) {
    client.loop();
  }

  // Adquire valores de temperatura e humidade
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float ecg_value = analogRead(ECG_PIN);
  float bpm = calculateBPM(ecg_value);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.print("Falha ao ler do sensor DHT 11");
    return;
  }

  // Cria json para envio
  StaticJsonDocument<128> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["timestamp"] = GetDateTime();
  doc["bpm"] = bpm;

  // Serialização do json
  char payload[100];
  serializeJson(doc, payload, sizeof(payload));

  // Verifica se perdeu a conexão com a internet ou com o RabbitMQ
  if (!CheckConnection()) {
    // Se perdeu a conexão, salva os dados localmente
    Serial.println("Sem conexão. Salvando dados localmente...");
    SaveToSPIFFS(payload);
    Reconnect();
  } else {
    // Publica na fila
    if (client.publish(mqtt_queue, payload)) {
      Serial.print("Enviado: ");
      Serial.println(payload);
    } else {
      // Caso houver erro na publicação, salva os dados localmente
      Serial.println("Falha ao publicar. Salvando localmente...");
      SaveToSPIFFS(payload);
    }

    // Após enviar com sucesso, tenta enviar os dados pendentes
    SendSavedData();
  }

  delay(100);
}