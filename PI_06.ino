// #include <WiFi.h>
// #include <WiFiClientSecure.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include <DHT.h>
// #include "secrets.h"

// --- DEFINIÇÃO DOS PINOS (PORTAS) DO ESP32 ---
#define PINO_DHT 4          // Pino digital onde o sensor DHT está conectado
#define PINO_UMIDADE_SOLO 34 // Pino analógico para o sensor de umidade do solo

// --- CONFIGURAÇÕES DOS SENSORES ---
#define TIPO_DHT DHT11

// --- CONFIGURAÇÕES AWS E MQTT ---
const char* TOPICO_PUBLICACAO_AWS = "esp32/data";
const int INTERVALO_PUBLICACAO = 30000; // Publicar a cada 30 segundos (30000 ms)

// --- VARIÁVEIS GLOBAIS ---
WiFiClientSecure redeSegura = WiFiClientSecure();
PubSubClient clienteMqtt(redeSegura);
DHT dht(PINO_DHT, TIPO_DHT);
unsigned long ultimoTempo = 0;

// --- CALIBRAÇÃO DO SENSOR DE UMIDADE DO SOLO ---
// Meça o valor com o sensor no ar (seco) e totalmente submerso na água para calibrar.
const int valorUmidadeSoloAr = 2800;   // Valor analógico para solo seco (exemplo)
const int valorUmidadeSoloAgua = 1300; // Valor analógico para solo 100% molhado (exemplo)

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Inicializa o sensor DHT
  dht.begin();

  // Conecta-se à rede Wi-Fi
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Conecta-se à AWS IoT Core
  conectarAWS();
}

void conectarAWS() {
  Serial.println("Conectando à AWS IoT...");

  // Configura a conexão segura (TLS) com os certificados necessários
  redeSegura.setCACert(AWS_CERT_CA);
  redeSegura.setCertificate(AWS_CERT_CRT);
  redeSegura.setPrivateKey(AWS_CERT_PRIVADO);

  // Informa ao cliente MQTT qual o servidor e a porta a se conectar
  clienteMqtt.setServer(AWS_IOT_ENDPOINT, 8883);

  // Tenta se conectar com o nome (ID do cliente) definido em secrets.h
  while (!clienteMqtt.connect(NOME_DA_COISA)) {
    Serial.print(".");
    delay(1000);
  }

  if (!clienteMqtt.connected()) {
    Serial.println("Falha na conexão com a AWS! Reiniciando...");
    ESP.restart(); // Reinicia o ESP32 em caso de falha grave
  }

  Serial.println("Conectado à AWS IoT!");
}

void publicarMensagem() {
  // 1. FAZ A LEITURA DOS SENSORES
  float umidadeAr = dht.readHumidity();
  float temperatura = dht.readTemperature(); // em Celsius

  
  int valorBrutoUmidadeSolo = analogRead(PINO_UMIDADE_SOLO);

  // Verifica se as leituras do sensor DHT são válidas (não são "NaN" - Not a Number)
  if (isnan(umidadeAr) || isnan(temperatura)) {
    Serial.println("Falha ao ler do sensor DHT!");
    return; // Sai da função se a leitura falhar
  }


  int percentualUmidadeSolo = map(valorBrutoUmidadeSolo, valorUmidadeSoloAr, valorUmidadeSoloAgua, 0, 100);

  percentualUmidadeSolo = constrain(percentualUmidadeSolo, 0, 100);

  // 2. MONTA A MENSAGEM NO FORMATO JSON
  StaticJsonDocument<200> doc;
  doc["temperatura"] = temperatura;
  doc["umidadeAr"] = umidadeAr;
  doc["umidadeSolo"] = percentualUmidadeSolo;
  doc["valorBrutoSolo"] = valorBrutoUmidadeSolo; // Envia o valor bruto para ajudar na calibração

  char bufferJson[512];
  serializeJson(doc, bufferJson); // Transforma o objeto JSON em texto

  // 3. PUBLICA A MENSAGEM NO TÓPICO MQTT
  if (clienteMqtt.publish(TOPICO_PUBLICACAO_AWS, bufferJson)) {
      Serial.print("Mensagem publicada com sucesso: ");
      Serial.println(bufferJson);
  } else {
      Serial.println("Falha ao publicar a mensagem!");
  }
}

void loop() {
 
  if (!clienteMqtt.connected()) {
    conectarAWS();
  }
  
 
  clienteMqtt.loop(); 

  if (millis() - ultimoTempo > INTERVALO_PUBLICACAO) {
    ultimoTempo = millis(); 
    publicarMensagem();
  }
}