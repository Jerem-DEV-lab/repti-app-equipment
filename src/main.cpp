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
#include <ArduinoJson.h>

#define DHTPIN 5         // Broche connectée au capteur DHT
#define DHTTYPE DHT11     // Type de capteur (DHT11 ou DHT22)


const char* ssid     = "S22";
const char* password = "Azertyui";
unsigned long previousMillis = 0;
const long interval = 1000;
const char* TEMP_MAX = "temp_max";

AsyncWebServer server(80);

const int relay = 4;
int relay_status = LOW;
DHT dht(DHTPIN, DHTTYPE);
float t = 0.0;
float h = 0.0;
int t_max = 0;

int critical_max = 200;

JsonDocument terraInfo;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="fr">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
          html {
           font-family: Arial;
           display: inline-block;
           margin: 0px auto;
           text-align: center;
          }
          h2 { font-size: 3.0rem; }
          p { font-size: 3.0rem; }
          .units { font-size: 1.2rem; }
          .dht-labels{
            font-size: 1.5rem;
            vertical-align:middle;
            padding-bottom: 15px;
          }
        </style>
      </head>
<body>
    <h2>ESP8266 DHT Server</h2>
</body>
<p>
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>

  <form id="form" class="temp-max-label">
    <label for="temp_max">Température max</label>
    <input id="temp_max" type="number" name="temp_max">
    <div id="error">
    </div>
    <button type="submit">Régler</button>
  </div>
</body>
<script>
const form = document.getElementById("form")

if(!form) {
  console.error('Formulaire introuvable')
}else {
  form.addEventListener("submit", (e) => {
    e.preventDefault();
    const inputTemp = document.getElementById('temp_max')
    const formData = new FormData();

    formData.append('temp_max', inputTemp.value);
    fetch('/set_temperature', {
      method: 'POST',
      body: formData,
      'Content-Type': 'multipart/form-data'
    })
  })
}
setInterval(async function ( ) {
    try {
        const request = await fetch('/temperature')
        const response = await request.json();
        if(request.ok){
            document.getElementById("temperature").innerHTML = response.temperature.current_temp + "| MAX" + response.temperature.max_temp;
        }
    }catch(e){
      document.getElementById("error").innertHTML = e.response;
        console.error('Impossible de récupérer, la température');
    }
}, 1000 ) ;
</script>
</html>)rawliteral";

String processor(const String& var){
  Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }else if (var == "T_MAX") {
    Serial.println(t_max);
    return String(t_max);  // Assure-toi que T_MAX est bien défini
  }
  return String();
}

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  Serial.print("Setting AP (Access Point)…");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  { 
    delay(1000); 
    Serial.print("."); 
  }

  Serial.print(WiFi.localIP());
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/temperature",HTTP_GET, [](AsyncWebServerRequest *request) {
    bool currentStateRelay = analogRead(relay);
    terraInfo["current_temp"] = t;
    terraInfo["max_temp"] = t_max;
    terraInfo["is_start"] = currentStateRelay;
    Serial.println(currentStateRelay);
    String payload;
    serializeJson(terraInfo, payload);
    request->send(200, "application/json", payload);
  });

  server.on("/set_temperature", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
        if (request->hasParam(TEMP_MAX, true)) {
            message = request->getParam(TEMP_MAX, true)->value();

            if(message.toInt() > critical_max){
              request->send(401, "text/plain", "Le seuil max est de " + critical_max);
            } else {
              t_max = request->getParam(TEMP_MAX, true)->value().toInt();
            }
        } else {
            message = "No temp max sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message);
  });
  dht.begin();
  server.begin();
}

void loop() {
   unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);

      if(newT <= t_max){
        digitalWrite(relay, HIGH);
      } else if(newT > t_max) {
        digitalWrite(relay, LOW);
      }
    }
   
  }
  delay(2000);  // Attente de 2 secondes avant la prochaine lecture
}
