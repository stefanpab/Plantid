#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebSrv.h>
#include <ArduinoOTA.h>

#define moisturePIN 34
#define temperaturePIN 35
#define pumpPIN 25
#define pause 10000
#define threshold 25.0

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char* ssid = "LifanHome";
const char* password = "nexapote2620!";

float moisture = 0;
float temperature = 0;

bool state = false;

void notFound(AsyncWebServerRequest *request) {
  String url = request->url();
  int slashPos = url.indexOf("/");
  String file = url.substring(slashPos + 1);
  
  if((SPIFFS.exists(file))) {
    request->send(SPIFFS, "/index.html");
  } else {
    request->send(404, "text/plain", "Not found"); 
  }
}

String readMoisture() {
  float moistvoltage = (analogRead(moisturePIN)/1024.0);
  //Serial.print("analog moist");
  //Serial.println(analogRead(moisturePIN));
  moisture = ((moistvoltage*50.0)/3.0); //converting incoming voltage value in %
  String moistureVal = String(moisture);
  /*Serial.print("float moist: ");
  Serial.println(moisture);
  Serial.print("String moist: ");
  Serial.println(moistureVal);*/ 
  if (isnan(moisture)) {
    Serial.println("Could not read any moisture value!");
    return "";
  } else {
    return moistureVal;
  }
}

String readTemp() {
  float tempvoltage = (analogRead(temperaturePIN)/1024.0);
  //Serial.print("analog temp");
  //Serial.println(analogRead(temperaturePIN));
  temperature = ((tempvoltage-0.5)*100.0); //converting incoming voltage value in °C
  String temperatureVal = String(temperature);
  /*Serial.print("float temp: ");
  Serial.println(temperature);
  Serial.print("String temp: ");
  Serial.println(temperatureVal);*/  
  if (isnan(temperature)) {
    Serial.println("Could not read any temperature value!");
    return "";
  } else {
    return temperatureVal;
  }
}

String readState() {
  String pumpState;
  if(state = true) {
    pumpState = "Pump active";
  } else {
    pumpState = "Pump inactive";
  }
}

void startPump() {
  digitalWrite(pumpPIN, LOW);
  if(moisture <= threshold) {
    Serial.println("Pump starts watering!");
    digitalWrite(pumpPIN, HIGH);
    state = true;
    delay(pause);
    Serial.println("Pump finished watering!");
    digitalWrite(pumpPIN, LOW);
    state = false;
  }
}

void setup() {
  // Serial port for debugging
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  ArduinoOTA.setHostname("plantid");
  ArduinoOTA.setPassword("admin");

  ArduinoOTA.begin();

  // Print ESP32 Local IP Address
  Serial.print("Local IP of ESP32: ");
  Serial.println(WiFi.localIP());

  // Initialize SPIFFS
  SPIFFS.begin();

  pinMode(moisturePIN, INPUT);
  pinMode(temperaturePIN, INPUT);
  pinMode(pumpPIN, OUTPUT);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemp().c_str());
  });
  
  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readMoisture().c_str());
  });

  server.on("/pumpState", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readState().c_str());
  });

  // if no server is found
  server.onNotFound(notFound);

  // Start server
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
}
