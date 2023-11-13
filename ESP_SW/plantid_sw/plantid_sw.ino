#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebSrv.h>
#include <ArduinoOTA.h>

#define moisturePIN 34
#define temperaturePIN 35
#define pumpPIN 25
#define pause 10000

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char* ssid = "LifanHome";
const char* password = "nexapote2620!";

float moisture = 0;
float temperature = 0;

bool state = false;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

String readMoisture() {
  double voltage = (analogRead(moisturePIN)*3.3)/1024;
  moisture = (voltage / 3) * 50; //converting incoming voltage value in %
  String moistureVal = String(moisture);
  Serial.println("Int: " + moisture);
  Serial.println("String: " + moistureVal);
  if (isnan(moisture)) {
    Serial.println("Could not read any moisture value!");
    return "";
  } else {
    return moistureVal;
  }
}

String readTemperature() {
  double voltage = (analogRead(temperaturePIN)*3.3)/1024;
  temperature = (voltage - 0.5) / (0.01); //converting incoming voltage value in °C
  String temperatureVal = String(temperature);
  Serial.println("Int: " + temperature);
  Serial.println("String: " + temperatureVal);
  if (isnan(temperature)) {
    Serial.println("Could not read any temperature value!");
    return "";
  } else {
    return temperatureVal;
  }
}

void startPump() {
  digitalWrite(pumpPIN, LOW);
  if(moisture <= 25.0) {
    Serial.println("Pump starts watering!");
    digitalWrite(pumpPIN, HIGH);
    delay(pause);
    Serial.println("Pump finished watering!");
    digitalWrite(pumpPIN, LOW);
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
    request->send_P(200, "text/plain", readTemperature().c_str());
  });
  
  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readMoisture().c_str());
  });

  // if no server is found
  server.onNotFound(notFound);

  // Start server
  server.begin();
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();
}
