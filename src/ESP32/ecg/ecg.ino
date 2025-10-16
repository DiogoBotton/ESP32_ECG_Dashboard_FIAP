#define ECG_PIN 36       // Pino de saída analógica do AD8232 (OUTPUT)
#define LO_PLUS_PIN 25   // Lead-off positivo (LO+)
#define LO_MINUS_PIN 26  // Lead-off negativo (LO-)

void setup() {
  Serial.begin(115200);

  pinMode(LO_PLUS_PIN, INPUT);
  pinMode(LO_MINUS_PIN, INPUT);
  pinMode(ECG_PIN, INPUT);

  Serial.println("Iniciando leitura do ECG...");
  delay(2000);
}

void loop() {
  // Verifica se os eletrodos estão conectados
  if (digitalRead(LO_PLUS_PIN) == 1 || digitalRead(LO_MINUS_PIN) == 1) {
    Serial.println(0); // Envia zero se os eletrodos estiverem soltos
  } else {
    int ecgValue = analogRead(ECG_PIN);
    Serial.println(ecgValue); // Envia o valor analógico para o Serial Plotter
  }

  delay(5); // Ajuste fino da taxa de amostragem (~200 Hz)
}