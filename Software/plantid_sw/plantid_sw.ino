#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebSrv.h>

#define moisturePIN 34
#define temperaturePIN 35
#define pumpPIN 25
#define pause 10000
#define threshold 25.0
#define uS_to_S 1000000
#define sleeptime 10

RTC_DATA_ATTR int dayboots = 0;
RTC_DATA_ATTR bool state = false;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char* ssid = "LifanHome";
const char* password = "nexapote2620!";

float moisture = 0;
float temperature = 0;

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
  if(moisture <= threshold && dayboots >= 8640) { //8640 boots is one day (24h)
    Serial.println("Pump starts watering!");
    digitalWrite(pumpPIN, HIGH);
    state = true;
    delay(pause);
    Serial.println("Pump finished watering!");
    digitalWrite(pumpPIN, LOW);
    state = false;
    dayboots = 0;
  }
}

void setup() {
  //Increment after every boot
  ++dayboots;

  //wakeup every 10 seconds
  esp_sleep_enable_timer_wakeup(sleeptime * uS_to_S); 
  
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

  //go to sleep
  esp_deep_sleep_start();
}

void loop() {
}
