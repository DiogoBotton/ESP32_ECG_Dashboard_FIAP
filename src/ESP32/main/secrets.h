#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID "Nome da rede aqui"
#define WIFI_PWD "Senha da rede aqui"

// Exemplo de configuração para broker MQTT local (RabbitMQ)
#define MQTT_IP "192.168.15.5" // Aqui deve vir o ip local da sua rede wifi
#define MQTT_PORT 1883 // Porta MQTT
#define MQTT_USER "guest"
#define MQTT_PWD "guest"
#define MQTT_QUEUE "dht11_ad8232" // Nome da fila. OBS: não pode ter barra /, evitar caracteres especiais

// Exemplo de configuração para HiveMQ Cloud
// #define MQTT_IP "<id da rede aqui>.s1.eu.hivemq.cloud" // Aqui deve vir o endereço do cluster do HiveMQ
// #define MQTT_PORT 8883 // Porta MQTT
// #define MQTT_USER "hivemq"
// #define MQTT_PWD "HiveMQ8883"
// #define MQTT_QUEUE "dht11_ad8232"

#endif