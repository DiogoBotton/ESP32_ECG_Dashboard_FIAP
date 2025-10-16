## Armazenamento local e envio de dados via protocolo MQTT

### Disposição dos pinos do sensor AD8232

| AD8232 | ESP32 WROOM | Observação                          |
| ------ | ----------- | ----------------------------------- |
| 3.3V   | 3V3         | Alimentação                         |
| GND    | GND         | Terra                               |
| OUTPUT | GPIO 34     | Entrada analógica (somente leitura) |
| LO+    | GPIO 25     | Digital                             |
| LO–    | GPIO 26     | Digital                             |
