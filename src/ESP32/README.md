## Armazenamento local e envio de dados via protocolo MQTT

### Disposição dos pinos dos sensores

| Sensor/Dispositivo | Pino Sensor  | Pino ESP32 WROOM | Observação                                     |
| ------------------ | ------------ | ---------------- | ---------------------------------------------- |
| **AD8232**         | 3.3V         | 3V3              | Alimentação                                    |
|                    | GND          | GND              | Terra                                          |
|                    | OUTPUT       | GPIO 34          | Entrada analógica (somente leitura)            |
|                    | LO+          | GPIO 25          | Detecção de eletrodo solto (opcional)          |
|                    | LO–          | GPIO 26          | Detecção de eletrodo solto (opcional)          |
| **DHT11**          | VCC          | 3V3              | Alimentação                                    |
|                    | GND          | GND              | Terra                                          |
|                    | DATA         | GPIO 19          | Comunicação digital com o ESP32                |
|                    | **Resistor** | DATA ↔ 3V3       | Resistor de **10kΩ pull-up** entre DATA e 3.3V |

### Diagramas e foto do projeto com ESP32 físico

**Diagrama AD8232**
![ad8232](readme-imgs/ad8232.png)

**Diagrama DHT11**
![dht11](readme-imgs/dht11.png)

**Projeto final**
![ESP32 fisico](readme-imgs/esp32.jpg)

**Vídeo do funcionamento do projeto**

[Funcionamento ESP32 + AD8232 + RabbitMQ/HiveMQ + Node RED](https://youtu.be/8xiXCnkW8kI)

## 🔧 Como executar o código

A fazer...

TODO:

- Dizer sobre como foi possível realizar a conexão do hiveMQ (wifiClientSecure).
- Falar sobre a url de conexão do rabbit mq local (necessidade de ser o nome do container) no node-RED e a importância de criar uma rede entre node-RED e RabbitMQ no docker-compose.
- Chave .pem para certificado TLS (falar caso necessite realizar conexão com HiveMQ).
- Fazer os relatórios da parte 1 e parte 2.
