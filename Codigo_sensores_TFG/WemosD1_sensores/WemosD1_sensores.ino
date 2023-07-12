#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h> 
#include <ArduinoJson.h>

// Configuración de la red WiFi
const char* ssid = "vodafone74E0";
const char* password = "TaralamasdivA99,";

// Configuración del broker MQTT
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;
const char* mqtt_topic = "datosSensores/sala2";

// Configuración del sensor DHT22
#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Configuración del sensor MQ-135
const int sensorPin = A0;  // Conexión analógica del sensor al pin A0
//Configuracion del sensor de luz bh1750
BH1750 Luxometro; 

// Cliente WiFi y MQTT
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);
  dht.begin();
  //Sensor de luz
  Wire.begin();
   Serial.println("Inicializando sensores...");
   Luxometro.begin(BH1750::CONTINUOUS_HIGH_RES_MODE); //inicializamos el sensor
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  float humidity = roundf(dht.readHumidity());
  float temperature = roundf(dht.readTemperature());

  if (isnan(temperature)) {
    Serial.println("Error al leer la temperatura del sensor DHT22");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }

  if (isnan(humidity)) {
    Serial.println("Error al leer la humedad del sensor DHT22");
  } else {
    Serial.print("Humedad: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  float sensorValueCO2 = analogRead(sensorPin);
  float sensorValueCO = analogRead(sensorPin);
  float ppmCO2 = getPPM(sensorValueCO2, 200, 1.0);
  float ppmCO = getPPM(sensorValueCO, 100, 2.0);

  Serial.print("CO2: ");
  Serial.print(ppmCO2);
  Serial.print(" ppm");
  Serial.print(", CO: ");
  Serial.print(ppmCO);
  Serial.println(" ppm");
  
//Sensor de luz
  uint16_t lux = Luxometro.readLightLevel();//Realizamos una lectura del sensor
  if(isnan(lux)){
     Serial.print("¡Error a la hora de leer el sensor de luz!");
  }else{
      Serial.print("Iluminacion: ");
      Serial.print(lux);
    }
  publishData(temperature, humidity, lux, ppmCO2, ppmCO);

  delay(60000);
}
// Función de conexión WiFi
void connectWiFi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conexión WiFi establecida");
  Serial.println("Dirección IP: " + WiFi.localIP().toString());
}

// Función de conexión MQTT
void connectMQTT() {
  while (!client.connected()) {
    Serial.println("\n Conectando al broker MQTT...");
    if (client.connect("ArduinoClient")) {
      Serial.println("Conexión MQTT establecida");
    } else {
      Serial.print("Error al conectar al broker MQTT - Estado: ");
      Serial.print(client.state());
      Serial.println(" Intentando nuevamente en 5 segundos...");
      delay(5000);
    }
  }
}

// Función para publicar los datos en MQTT
void publishData(float temperature, float humidity, float lux, float ppmCO2, float ppmCO) {
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["lux"] = lux;
  jsonDoc["CO2"] = ppmCO2;
  jsonDoc["CO"] = ppmCO;

  char buffer[200];
  serializeJson(jsonDoc, buffer);

  client.publish(mqtt_topic, buffer);
}
float getPPM(float sensorValue, float zeroValue, float correctionFactor) {
  float ppm = (sensorValue - zeroValue) * correctionFactor;
  return ppm;
}
