/*
Project:  Plantid - Automatic Watering for Plants
Author:   Stefan Pabisch (sa21b004@technikum-wien.at)
Date:     Created 19.04.2020 
Version:  V1.0
 
In this project it will be possible to automatically water your plants. In Addition
to that, there will be a web gui to monitor the temperature, moisture, pump state
and the current watering time. In the web gui it is possible to manually change the
watering time.
*/

#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebSrv.h>
#include <ESP32Time.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

#define moisturePIN 34 // PIN for moisture value
#define temperaturePIN 35 // PIN for temperature value
#define manuell_PUMP 0 // ESP32 Button which manually starts the pump
#define pumpPIN 25 // MOSFET PIN to activate pump
#define threshold 25.0 // threshold for moisture to start watering (threshhold in prozent)

uint32_t wateringTime = 10000; // watering time in milliseconds
int manuell_btn_state = 0; // button state of onboard button from esp32 to start pump manually
int lastManuellButtonState = 0; // last manuell state of button
uint32_t manuell_btn_pressTime = 0; // time how long manuell watering button is pressed

RTC_DATA_ATTR uint8_t currentDay = 0; // save current day in RTC to check if it is a new day

bool isNewDay = false; 
bool currentPumpState = false; // checks if pump is active at the moment or not

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// ESP32Time rtc;
ESP32Time rtc(3600);  // offset in seconds GMT+1

// Create preferences
Preferences preferences;

//Home WIFI
const char* ssid = "LifanHome";
const char* password = "nexapote2620!";

//extern WIFI
const char* ssidExt = "aat-technikum2.4GHz";
const char* passwordExt = "2AATractive$$";

float moisture = 0; // save read moisture value
float temperature = 0; // save read temperature value

/*
 * This function handles if server cant be found.
 */
void notFound(AsyncWebServerRequest *request) {
  String url = request->url(); // get url 
  int slashPos = url.indexOf("/"); // find first slash
  String file = url.substring(slashPos + 1); // save as string everything after slash

  if ((SPIFFS.exists(file))) {
    request->send(SPIFFS, file);
  } else {
    request->send(404, "text/plain", "Not found");
  }
}

/*
 * This function is to check which day it is right now so the plants will only be watered once a day.
 */
void getCurrentDay() {
  int day = rtc.getDay();
  if (day != currentDay) {
    isNewDay = true;
  } else {
    isNewDay = false;
  }
}

/*
 * This function is to read out the moisture from the SMT50 sensor. It will then be saved in a string which gets returned.
 */
String readMoisture() {
  float moistvoltage = (analogRead(moisturePIN) / 1024.0);
  moisture = ((moistvoltage * 50.0) / 3.0); //converting incoming voltage value in %
  String moistureVal = String(moisture);
  if (isnan(moisture)) {
    Serial.println("Could not read any moisture value!");
    return "";
  } else {
    return moistureVal;
  }
}

/*
 * This function is to read out the temperature from the SMT50 sensor. It will then be saved in a string which gets returned.
 */
String readTemp() {
  float tempvoltage = (analogRead(temperaturePIN) / 1024.0);
  temperature = ((tempvoltage - 0.5) * 100.0); //converting incoming voltage value in °C
  String temperatureVal = String(temperature);
  if (isnan(temperature)) {
    Serial.println("Could not read any temperature value!");
    return "";
  } else {
    return temperatureVal;
  }
}

/*
 * This function is to check in which state the pump is.
 */
String readPumpState() {
  String pumpStr;
  if (currentPumpState == true) {
    pumpStr = "Pump active";
  } else {
    pumpStr = "Pump inactive";
  }
  return pumpStr;
}

/*
 * This function will activate the pump and start the watering process. If the threshold from 25% is reached
 * and not already a new day is it will start the pump for standard 10 seconds. In addition it changes the pump
 * state during active pumping.
 */
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

  // Start prefernces and set a standard value for the watering time to 10 seconds
  preferences.begin("plantid-pref", false);
  wateringTime = preferences.getInt("wateringTime", 10000);

  // Serial port for debugging
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // set wifi mode

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

  // set sensors as input and pump as output
  pinMode(moisturePIN, INPUT);
  pinMode(temperaturePIN, INPUT);
  pinMode(pumpPIN, OUTPUT);
  pinMode(manuell_PUMP, INPUT);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });

  // Route for temperature value
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readTemp().c_str());
  });

  // Route for moisture value
  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readMoisture().c_str());
  });

  // Route for pump state
  server.on("/pumpState", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readPumpState().c_str());
  });

  // Route to get the current watering time
  server.on("/getWateringTime", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"wateringTime\":" + String(wateringTime) + "}";
    request->send(200, "application/json", json);
  });

  // Route to update the watering time
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

  // if no server is found call not found function
  server.onNotFound(notFound);

  // Start server
  server.begin();

  // close preferences
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
