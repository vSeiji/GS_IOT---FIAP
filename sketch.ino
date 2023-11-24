//Adicionando bibliotecas
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
 
//Definindo variaveis para: DHT22 e Buzzer
#define led 17
#define led2 5
#define buzzer 2
#define dhtpin  4  
#define dhttype DHT22
int bpm = 70;
 
//Definindo variaveis para o Display Led
#define i2c_adr 0x27
#define lcd_colunas 16
#define lcd_linhas 2

// Essas constantes devem corresponder aos atributos "gama" e "rl10" do fotoresistor
const float GAMMA = 0.7;
const float RL10 = 50;
 
//Variaveis para o MQTT Broker
char* mqttServer = "broker.hivemq.com";
int mqttPort = 1883;
char* clientId = "usuarioTeste";

//Variaveis dos topicos MQTT
char* topicTemp = "GS-IOT/projeto/temp";
char* topicUmid = "GS-IOT/projeto/umid";
char* topicBpm = "GS-IOT/projeto/bpm";
char* topicLux = "GS-IOT/projeto/lux";
 
//Variaveis para ligar o Wi-fi do Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

//Definindo variaveis do tipo das bibliotecas
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(dhtpin, dhttype);
LiquidCrystal_I2C lcd(i2c_adr, lcd_colunas, lcd_linhas);
 
void setup() {
  Serial.begin(115200);

  //Inicializando hardware
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  dht.begin();
  lcd.init();
  lcd.backlight();

  //Conectando ao Wi-fi
  WiFi.begin(ssid, password, 6);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.println("Conectando ao WiFi...");
  }
  Serial.println("Conectado ao WiFi");
 
  //Conectando aos topicos do MQTT Broker
  client.setServer(mqttServer, mqttPort);
 
  if (client.connect(clientId)) {
    Serial.println("Conectado ao Broker MQTT");
    client.subscribe(topicTemp);
    client.subscribe(topicUmid);
    client.subscribe(topicBpm);
    client.subscribe(topicLux);
  } else {
    Serial.println("Falha na conexão ao Broker MQTT");
  }
}
 
void loop() {
 
  float umidade = dht.readHumidity();
  float temp = dht.readTemperature();

  // Converte o valor analógico em valor lux:
  int analogValue = analogRead(34);
  float voltage = analogValue / 1024. * 5;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));

  if (bpm == 200) {
    bpm = 70;
  }

  bpm += 10;

  notifLed(bpm);
  notifLux(bpm, lux);
  
  char tempStr[10];
  char umidadeStr[10];
  char bpmStr[10];
  char luxStr[10];
 
  dtostrf(temp, 1, 2, tempStr);
  dtostrf(umidade, 1, 2, umidadeStr);
  dtostrf(bpm, 1, 2, bpmStr);
  dtostrf(lux, 1, 2, luxStr);
 
  client.publish(topicTemp, tempStr);
  client.publish(topicUmid, umidadeStr);
  client.publish(topicBpm, bpmStr);
  client.publish(topicLux, luxStr);
   
  client.loop();
 
}

void myTone( int pin)
{
  ledcAttachPin(pin, 0);
  ledcWriteNote(0, NOTE_F, 4);
}

void myNoTone( int pin)
{
  ledcDetachPin(pin);
}

void notifLux(int bpmP, int luxP) {
  if((bpmP < 100) && (luxP < 40))
  {
    myTone(buzzer);
    digitalWrite(led2, HIGH);
    delay(500);
  }
  else if (bpmP > 160 && luxP < 40)
  {
    myTone(buzzer);
    digitalWrite(led2, HIGH);
    delay(500);
  }
  else
  {
    myNoTone(buzzer);
    digitalWrite(led2, LOW);
    delay(500);
  }
}

void notifLed(int bpm2) {
  String teste;
  lcd.setCursor(0, 1);
  lcd.print("BPM: ");
  if(bpm2 < 100)
  {
    teste = "WARNING!";
    lcd.print(bpm2);
    lcd.print("          ");
    lcd.setCursor(0, 0);
    lcd.print(teste);
    digitalWrite(led, HIGH);
    delay(500);
  }
  else if (bpm2 > 160)
  {
    teste = "WARNING!";
    lcd.print(bpm2);
    lcd.print("          ");
    lcd.setCursor(0, 0);
    lcd.print(teste);
    digitalWrite(led, HIGH);
    delay(500);
  }
  else
  {
    teste = "Ok!";
    lcd.print(bpm2);
    lcd.print("          ");
    lcd.setCursor(0, 0);
    lcd.print(teste);
    lcd.setCursor(3, 0);
    for (int i = 3; i <=8; i++) {
      lcd.print(" ");
    }
    digitalWrite(led, LOW);
    delay(500);
  }
}
