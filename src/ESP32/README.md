## Armazenamento local e envio de dados via protocolo MQTT

### Disposição dos pinos do sensor AD8232

| AD8232 | ESP32 WROOM | Observação                          |
| ------ | ----------- | ----------------------------------- |
| 3.3V   | 3V3         | Alimentação                         |
| GND    | GND         | Terra                               |
| OUTPUT | GPIO 34     | Entrada analógica (somente leitura) |
| LO+    | GPIO 25     | Digital                             |
| LO–    | GPIO 26     | Digital                             |

TODO:

- Fazer disposição dos pinos do sensor DHT 11.
- Dizer sobre como foi possível realizar a conexão do hiveMQ (wifiClientSecure).
- Falar sobre a url de conexão do rabbit mq local (necessidade de ser o nome do container) no node-RED e a importância de criar uma rede entre node-RED e RabbitMQ no docker-compose.
- Fazer os relatórios da parte 1 e parte 2.
