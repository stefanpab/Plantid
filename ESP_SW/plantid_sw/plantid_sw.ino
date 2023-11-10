#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebSrv.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char* ssid = "LifanHome";
const char* password = "nexapote2620!";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

#define moisturePIN 34
#define temperaturePIN 35

int moisture = 0;
int temperature = 0;


String readMoisture() {
  moisture = analogRead(moisturePIN);
  moisture = (moisture / 3) * 50; //converting incoming voltage value in %
  String moistureVal = String(moisture);
  Serial.println("Int: " + moisture);
  Serial.println("String: " + moistureVal);
  if (moisture == 0) {
    Serial.println("Could not read any moisture value!");
    return "";
  } else {
    return moistureVal;
  }
}

String readTemperature() {
  temperature = analogRead(temperaturePIN);
  temperature = (temperature - 0.5) / (0.01); //converting incoming voltage value in °C
  String temperatureVal = String(temperature);
  Serial.println("Int: " + temperature);
  Serial.println("String: " + temperatureVal);
  if (temperature == 0) {
    Serial.println("Could not read any temperature value!");
    return "";
  } else {
    return temperatureVal;
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

  // Print ESP32 Local IP Address
  Serial.print("Local IP of ESP32: ");
  Serial.println(WiFi.localIP());

  // Initialize SPIFFS
  SPIFFS.begin();

  pinMode(moisturePIN, INPUT);
  pinMode(temperaturePIN, INPUT);

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
  //will never be used
}
