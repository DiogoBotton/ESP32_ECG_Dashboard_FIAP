#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "secrets.h"

// Definição das constantes
const char* wifi_ssid = WIFI_SSID;
const char* wifi_pwd = WIFI_PWD;

const char* mqtt_ip = MQTT_IP;
const int mqtt_port = MQTT_PORT;
const char* mqtt_user = MQTT_USER;
const char* mqtt_pwd = MQTT_PWD;
const char* mqtt_queue = MQTT_QUEUE;

// Definição do pino e tipo do DHT
#define DHTPIN 19
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Declarando variaveis de Wifi e MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  // Conecta ao Wifi e RabbitMQ
  Reconnect();
  
  dht.begin();
  delay(2000);
}

void Reconnect(){
  ConnectWifi();
  ConnectRabbitMQ();
}

void ConnectWifi(){
  WiFi.begin(wifi_ssid, wifi_pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado ao Wifi.");
}

void ConnectRabbitMQ(){
  client.setServer(mqtt_ip, mqtt_port);

  String client_id = "esp32-client-" + String(WiFi.macAddress());
  // Conectar ao RabbitMQ
  while (!client.connected()) {
    Serial.println("Conectando ao RabbitMQ...");

    if (client.connect(client_id.c_str(), mqtt_user, mqtt_pwd)) {
      Serial.println("Conectado ao RabbitMQ via MQTT");
      client.subscribe(mqtt_queue);
      client.publish(mqtt_queue, "Estou conectado :)");
    } else {
      Serial.print("Falha na conexão, código: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void loop() {
  // Caso perder a conexão com o RabbitMQ, tenta se reconectar novamente ao Wifi e Rabbit
  if (!client.connected()) {
    Reconnect();
  }

  client.loop();

  // Adquire valores de temperatura e humidade
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.print("Falha ao ler do sensor DHT 11");
    return;
  }

  // Cria json para envio
  char payload[100];
  snprintf(payload, sizeof(payload), "{\"temperature\": %.2f, \"humidity\": %.2f}", temperature, humidity);

  // Publica na fila
  if (client.publish(mqtt_queue, payload)) {
    Serial.print("Enviado: ");
    Serial.println(payload);
  } else {
    Serial.println("Falha ao publicar mensagem no MQTT");
  }

  delay(5000);
}
