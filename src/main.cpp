#include <Arduino.h>

#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <WiFi.h>

#include <WiFiClientSecure.h>

#include <PubSubClient.h>
 
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
// Wi-Fi

const char* ssid = "laryssa mesquita";

const char* password = "28320770";
 
// MQTT - HiveMQ Cloud

const char* mqtt_server = "2e1fa40f1cfb4799b90b3c664651150e.s1.eu.hivemq.cloud";

const int mqtt_port = 8883;
 
const char* mqtt_user = "nupreco";

const char* mqtt_password = "Admin123456";
 
const char* codigo_serial = "LCD-02";

const char* mqtt_topic_prefix = "nupreco/etiquetas";

char mqtt_topic[80];
 
WiFiClientSecure espClient;

PubSubClient client(espClient);
 
// Botões

#define BT1 35

#define BT2 32

#define BT3 27

#define BT4 25

#define BT5 26
 
bool estadoAnteriorBT1 = LOW;

bool estadoAnteriorBT2 = LOW;

bool estadoAnteriorBT3 = LOW;

bool estadoAnteriorBT4 = LOW;

bool estadoAnteriorBT5 = LOW;
 
String produtoExibido = "";

float precoExibido = 0.0;
 
int offsetScroll = 0;

unsigned long timerScroll = 0;
 
void mostrarProduto(String nome, float preco);

void atualizarScroll();

void conectarMQTT();

void callback(char* topic, byte* payload, unsigned int length);
 
void setup() {

  Serial.begin(115200);
 
  lcd.init();

  lcd.backlight();
 
  pinMode(BT1, INPUT);

  pinMode(BT2, INPUT);

  pinMode(BT3, INPUT);

  pinMode(BT4, INPUT);

  pinMode(BT5, INPUT);
 
  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("Conectando WiFi");

  Serial.println("Iniciando conexao WiFi...");
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    lcd.setCursor(0, 1);

    lcd.print("Aguarde...      ");

    Serial.println("Tentando conectar...");

  }
 
  Serial.println("WiFi conectado!");

  Serial.print("IP do ESP32: ");

  Serial.println(WiFi.localIP());
 
  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("WiFi Conectado");

  lcd.setCursor(0, 1);

  lcd.print(WiFi.localIP().toString());

  delay(2500);
 
  snprintf(

    mqtt_topic,

    sizeof(mqtt_topic),

    "%s/%s",

    mqtt_topic_prefix,

    codigo_serial

  );
 
  espClient.setInsecure();
 
  client.setServer(mqtt_server, mqtt_port);

  client.setCallback(callback);
 
  conectarMQTT();
 
  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("   BEM VINDO   ");

  lcd.setCursor(0, 1);

  lcd.print("Aguardando...");

}
 
void loop() {

  if (!client.connected()) {

    conectarMQTT();

  }
 
  client.loop();
 
  bool estadoBT1 = digitalRead(BT1);

  bool estadoBT2 = digitalRead(BT2);

  bool estadoBT3 = digitalRead(BT3);

  bool estadoBT4 = digitalRead(BT4);

  bool estadoBT5 = digitalRead(BT5);
 
  if (estadoBT1 == HIGH && estadoAnteriorBT1 == LOW) {

    mostrarProduto("Acucar Refinado Uniao 1kg", 4.25);

  }
 
  if (estadoBT2 == HIGH && estadoAnteriorBT2 == LOW) {

    mostrarProduto("Oleo de Soja Soya 900ml", 6.15);

  }
 
  if (estadoBT3 == HIGH && estadoAnteriorBT3 == LOW) {

    mostrarProduto("Agua Mineral Crystal 500ml", 2.50);

  }
 
  if (estadoBT4 == HIGH && estadoAnteriorBT4 == LOW) {

    mostrarProduto("Cerveja Skol Lata 350ml", 5.99);

  }
 
  if (estadoBT5 == HIGH && estadoAnteriorBT5 == LOW) {

    mostrarProduto("Farofa Pronta Yoki 400g", 7.80);

  }
 
  estadoAnteriorBT1 = estadoBT1;

  estadoAnteriorBT2 = estadoBT2;

  estadoAnteriorBT3 = estadoBT3;

  estadoAnteriorBT4 = estadoBT4;

  estadoAnteriorBT5 = estadoBT5;
 
  atualizarScroll();
 
  delay(50);

}
 
void conectarMQTT() {

  while (!client.connected()) {

    Serial.println("Conectando ao broker MQTT...");

    Serial.print("Broker: ");

    Serial.println(mqtt_server);

    Serial.print("Topico: ");

    Serial.println(mqtt_topic);
 
    lcd.clear();

    lcd.setCursor(0, 0);

    lcd.print("Conectando MQTT");

    lcd.setCursor(0, 1);

    lcd.print("Aguarde...");
 
    String clientId = "Nupreco-";

    clientId += String(codigo_serial);

    clientId += "-";

    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);
 
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {

      Serial.println("MQTT conectado!");
 
      client.subscribe(mqtt_topic);
 
      Serial.print("Inscrito no topico: ");

      Serial.println(mqtt_topic);
 
      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("MQTT Conectado");

      lcd.setCursor(0, 1);

      lcd.print(codigo_serial);

      delay(1500);

    } else {

      Serial.print("Falha MQTT. Codigo: ");

      Serial.println(client.state());
 
      lcd.clear();

      lcd.setCursor(0, 0);

      lcd.print("Falha MQTT");

      lcd.setCursor(0, 1);

      lcd.print("Cod: ");

      lcd.print(client.state());
 
      delay(3000);

    }

  }

}
 
void callback(char* topic, byte* payload, unsigned int length) {

  String mensagem = "";
 
  for (unsigned int i = 0; i < length; i++) {

    mensagem += (char)payload[i];

  }
 
  Serial.print("Mensagem recebida [");

  Serial.print(topic);

  Serial.print("]: ");

  Serial.println(mensagem);
 
  int separador = mensagem.indexOf(';');
 
  if (separador == -1) {

    Serial.println("Formato invalido. Use Nome;Preco");

    return;

  }
 
  String nome = mensagem.substring(0, separador);

  String precoTexto = mensagem.substring(separador + 1);
 
  nome.trim();

  precoTexto.trim();
 
  float preco = precoTexto.toFloat();
 
  mostrarProduto(nome, preco);

}
 
void mostrarProduto(String nome, float preco) {

  produtoExibido = nome;

  precoExibido = preco;

  offsetScroll = 0;

  timerScroll = 0;
 
  lcd.clear();
 
  lcd.setCursor(0, 1);

  lcd.print("Preco: R$ ");

  lcd.print(precoExibido, 2);
 
  Serial.print("Exibindo produto: ");

  Serial.print(nome);

  Serial.print(" | Preco: ");

  Serial.println(precoExibido, 2);

}
 
void atualizarScroll() {

  if (produtoExibido == "") return;
 
  if (millis() - timerScroll > 300) {

    timerScroll = millis();
 
    lcd.setCursor(0, 0);
 
    if (produtoExibido.length() > 16) {

      String msg = produtoExibido + "   ";
 
      lcd.print(msg.substring(offsetScroll, offsetScroll + 16));
 
      offsetScroll++;
 
      if (offsetScroll > msg.length() - 16) {

        offsetScroll = 0;

      }

    } else {

      lcd.print("                ");

      lcd.setCursor(0, 0);

      lcd.print(produtoExibido);

    }

  }

}
 