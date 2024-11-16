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


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
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

  <div class="temp-max-label">
    <label>Température max</label>
    <input type="number" value="%T_MAX%">
  </div>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
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
    request->send(200, "text/plain", String(t));
  });

  server.on("/set_temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    String message;
        if (request->hasParam(TEMP_MAX)) {
            message = request->getParam(TEMP_MAX)->value();
            t_max = request->getParam(TEMP_MAX)->value().toInt();
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
