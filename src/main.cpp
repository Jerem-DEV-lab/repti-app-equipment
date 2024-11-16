/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 5         // Broche connectée au capteur DHT
#define DHTTYPE DHT11     // Type de capteur (DHT11 ou DHT22)

const int relay = 4;
int relay_status = LOW;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  Serial.println("Démarrage");
  dht.begin();
}

void loop() {
  // Lecture de la température et de l'humidité
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Vérification des valeurs lues
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Échec de la lecture du capteur DHT !");
  } else if(temperature <= 25) {
      Serial.print("Température : ");
      Serial.println(temperature);
      digitalWrite(relay, HIGH);
      delay(1000);
  } else if(temperature >= 26) {
      Serial.print("Température : ");
      Serial.println(temperature);
      digitalWrite(relay, LOW);
  }

  
  delay(2000);  // Attente de 2 secondes avant la prochaine lecture
}
