#include "DHT.h"
#define DHTTYPE DHT22   // DHT 22  
#include <Wire.h>
#include <BH1750.h> 
//BH1750 luxómetro
BH1750 Luxometro; 
//Configuracion del sensor de temperatura y humedad
const int DHTPin = 6;     // PIN al que se conecta el DHT22
DHT dht(DHTPin, DHTTYPE); //Se define el sensor DHT22
// Configuración del sensor MQ-135
const int sensorPin = A0;  // Conexión analógica del sensor al pin A0


void setup() {
   Serial.begin(9600);
   //Sensor de temperatura y humedad
   dht.begin();
   //Sensor de luz
   Wire.begin();
   Serial.println("Inicializando sensor...");
   Luxometro.begin(BH1750::CONTINUOUS_HIGH_RES_MODE); //inicializamos el sensor

}

void loop() {
   // Lector de temperatura y humedad
   float h = dht.readHumidity();
   float t = dht.readTemperature();
  //Tener en cuenta el formato JSON para insertar en NODE-RED
   Serial.print("{\"temperatura\":");
  Serial.print(t);
  Serial.print(",\"humedad\":");
  Serial.print(h);

  //Sensor de luz
  uint16_t lux = Luxometro.readLightLevel();//Realizamos una lectura del sensor
   if(isnan(lux)){
     Serial.print("¡Error a la hora de leer el sensor de luz!");
  }
  Serial.print(",\"iluminacion\":");
  Serial.print(lux);

  //Sensor MQ-135
  float sensorValueCO2 = analogRead(sensorPin);
  float sensorValueCO = analogRead(sensorPin);
  float ppmCO2 = getPPM(sensorValueCO2, 200, 1.0);
  float ppmCO = getPPM(sensorValueCO, 100, 2.0);

  Serial.print(",\"CO\":");
  Serial.print(ppmCO);
  
  Serial.print(",\"CO2\":");
  Serial.print(ppmCO2);

  Serial.println("}");
  delay(60000);


}
float getPPM(float sensorValue, float zeroValue, float correctionFactor) {
  float ppm = (sensorValue - zeroValue) * correctionFactor;
  return ppm;
}
