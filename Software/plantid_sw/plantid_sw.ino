#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebSrv.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

#define moisturePIN 34
#define temperaturePIN 35
#define manuell_PUMP 0
#define pumpPIN 25
#define threshold 25.0
#define uS_to_S 1000000
#define sleeptime 10

uint32_t wateringTime = 10000;
int manuell_btn_state = 0;
int lastManuellButtonState = 0;
uint32_t manuell_btn_pressTime = 0;

RTC_DATA_ATTR uint8_t currentDay = 0;

bool isNewDay = false;
bool currentPumpState = false;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//ESP32Time rtc;
ESP32Time rtc(3600);  // offset in seconds GMT+1

Preferences preferences;

  //Home
  const char* ssid = "LifanHome";
  const char* password = "nexapote2620!";*/

//extern

const char* ssidExt = "aat-technikum2.4GHz";
const char* passwordExt = "2AATractive$$";

float moisture = 0;
float temperature = 0;

void notFound(AsyncWebServerRequest *request) {
  String url = request->url();
  int slashPos = url.indexOf("/");
  String file = url.substring(slashPos + 1);

  if ((SPIFFS.exists(file))) {
    request->send(SPIFFS, file);
  } else {
    request->send(404, "text/plain", "Not found");
  }
}

void getCurrentDay() {
  int day = rtc.getDay();
  if (day != currentDay) {
    isNewDay = true;
  } else {
    isNewDay = false;
  }
}

String readMoisture() {
  float moistvoltage = (analogRead(moisturePIN) / 1024.0);
  //Serial.print("analog moist");
  //Serial.println(analogRead(moisturePIN));
  moisture = ((moistvoltage * 50.0) / 3.0); //converting incoming voltage value in %
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
  float tempvoltage = (analogRead(temperaturePIN) / 1024.0);
  //Serial.print("analog temp");
  //Serial.println(analogRead(temperaturePIN));
  temperature = ((tempvoltage - 0.5) * 100.0); //converting incoming voltage value in °C
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

String readPumpState() {
  String pumpStr;
  if (currentPumpState == true) {
    pumpStr = "Pump active";
  } else {
    pumpStr = "Pump inactive";
  }
  return pumpStr;
}

void startPump() {
  delay(1000);
  digitalWrite(pumpPIN, LOW);
  if (moisture <= threshold && isNewDay == true) {
    currentDay = rtc.getDay();
    Serial.println("Pump starts watering!");
    digitalWrite(pumpPIN, HIGH);
    currentPumpState = true;
    Serial.println(currentPumpState);
    delay(wateringTime);
    Serial.println("Pump finished watering!");
    digitalWrite(pumpPIN, LOW);
    currentPumpState = false;
    Serial.println(currentPumpState);
  }
}

void setup() {

  preferences.begin("plantid-pref", false);
  wateringTime = preferences.getInt("wateringTime", 10000);

  // Serial port for debugging
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  /*
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }*/

  // Initialize mDNS
  if (!MDNS.begin("plantid")) {   // Set the hostname to "plantid.local"
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  //intialize WifiManager
  WiFiManager wm;

  bool wifi_connection;
  bool wifi_connection_ext;
  wifi_connection = wm.autoConnect(ssid,password); 

  if(!wifi_connection) {
      Serial.println("Failed to connect to LifanHome");
      wifi_connection_ext = wm.autoConnect(ssid,password);
      if(!wifi_connection_ext) {
        Serial.println("Failed to connect to Extern Wifi"); 
      } else {
        Serial.println("Connected to Extern Wifi");
      }
  } 
    else {
      Serial.println("Connected to LifanHome");
  }

  // Print ESP32 Local IP Address
  Serial.print("Local IP of ESP32: ");
  Serial.println(WiFi.localIP());

  // Initialize SPIFFS
  SPIFFS.begin();

  pinMode(moisturePIN, INPUT);
  pinMode(temperaturePIN, INPUT);
  pinMode(pumpPIN, OUTPUT);
  pinMode(manuell_PUMP, INPUT);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readTemp().c_str());
  });

  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readMoisture().c_str());
  });

  server.on("/pumpState", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readPumpState().c_str());
  });

/*
  server.on("/updateWateringTime", HTTP_GET, handleUpdatePreferences);

  server.on("/wateringTime", HTTP_GET, [](AsyncWebServerRequest * request) {
    preferences.begin("plantid-pref", false);
    uint32_t currentWateringTime = preferences.getUInt("wateringTime", wateringTime);
    String currentWateringTimeStr = String(currentWateringTime);
    preferences.end();
    request->send_P(200, "text/plain", currentWateringTimeStr.c_str());
  });*/

  //get the current watering time
  server.on("/getWateringTime", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"wateringTime\":" + String(wateringTime) + "}";
    request->send(200, "application/json", json);
  });

  //update the watering time
  server.on("/updateWateringTime", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("wateringTime", true)){
      uint32_t newWateringTime = request->getParam("wateringTime", true)->value().toInt();
      wateringTime = newWateringTime;

      //save it to preferences
      preferences.putInt("wateringTime", wateringTime);
      
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  // if no server is found
  server.onNotFound(notFound);

  // Start server
  server.begin();

  preferences.end();
}

void loop() {
  readMoisture();
  getCurrentDay();
  startPump();
  readPumpState();

  // get current state of button
  manuellButtonState = digitalRead(manuell_PUMP);

  // check if button is pressed
  if (manuellButtonState == HIGH && lastManuellButtonState == LOW) {
    manuell_btn_pressTime = millis();  // save pressed time
  }

  // check if button is released
  if (manuellButtonState == LOW && lastManuellButtonState == HIGH) {
    // check if the button is being pressed more then 500ms
    if (millis() - manuell_btn_pressTime >= 500) {
      digitalWrite(pumpPIN, HIGH); //start watering
      Serial.println("Pump starts watering!");
      currentPumpState = true;
    }
  }

  // refresh last state of button
  lastManuellButtonState = manuellButtonState;

  // check if watering ready to stop
  if (manuellButtonState == LOW) {
    Serial.println("Pump finished watering!");
    digitalWrite(pumpPIN, LOW);
    currentPumpState = false;
  }
}
