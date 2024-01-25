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

#define moisturePIN 34 // ADC PIN which reads the moisture value
#define temperaturePIN 35 // ADC PIN which reads the temperature value
#define manuell_PUMP 0 // ESP32 Button which manually starts the pump when it is pressed (EN button)
#define pumpPIN 25 // MOSFET PIN to activate pump
#define threshold 25.0 // threshold for moisture to start watering (threshhold in prozent)

uint32_t wateringTime = 10000; // standard watering time in milliseconds
int manuell_btn_state = 0; // button state of onboard button from esp32 to start pump manually, will be triggered when button is pressed
int lastManuellButtonState = 0; // last manual state of button
uint32_t manuell_btn_pressTime = 0; // time how long manuell watering button is pressed

RTC_DATA_ATTR uint8_t currentDay = 0; // save current day in RTC to check if it is a new day (0-31)

bool isNewDay = false; // bool variable to check if a new day is already reached
bool currentPumpState = false; // checks if pump is active at the moment or not

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// ESP32Time rtc;
ESP32Time rtc(3600);  // offset in seconds GMT+1

// Create preferences
Preferences preferences;

// WIFI credentials -> change to yours if you use it
const char* ssid = "LifanHome";
const char* password = "nexapote2620!";

float moisture = 0; // save read moisture value as a float value to calculate
float temperature = 0; // save read temperature value as a float value to calculate

/*
 * This function handles if server cant be found.
 */
void notFound(AsyncWebServerRequest *request) {
  String url = request->url(); // get current url 
  int slashPos = url.indexOf("/"); // search for the first slash
  String file = url.substring(slashPos + 1); // save everything as string after a slash is found

  if ((SPIFFS.exists(file))) {
    request->send(SPIFFS, file); // send the saved file which is already in the spiffs
  } else {
    request->send(404, "text/plain", "No file in SPIFFS found"); // send and error code, that no spiffs is found
  }
}

/*
 * This function is to check which day it is right now so the plants will only be watered once a day.
 */
void getCurrentDay() {
  int day = rtc.getDay(); // get the current day
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
  // read value from adc pin and divide it through 1024.0 to convert it to a voltage value, has to be
  // diveded through 1024.0 bc else there would be a problem converting bc of different data types
  float moistvoltage = (analogRead(moisturePIN) / 1024.0);  
  moisture = ((moistvoltage * 50.0) / 3.0); // converting incoming voltage value in %, this formular is from the producer of the sensor itself
  String moistureVal = String(moisture);
  // this if is required to see if there was any problem converting or reading the value
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
  // read value from adc pin and divide it through 1024.0 to convert it to a voltage value, has to be
  // diveded through 1024.0 bc else there would be a problem converting bc of different data types
  float tempvoltage = (analogRead(temperaturePIN) / 1024.0);
  temperature = ((tempvoltage - 0.5) * 100.0); // converting incoming voltage value in °C
  String temperatureVal = String(temperature);
  // this if is required to see if there was any problem converting or reading the value
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
  // through this if the current pump state variable will be changed which is needed to display it on the web gui
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
  delay(1000); // set a delay of one second to avoid errors
  digitalWrite(pumpPIN, LOW);
  if (moisture <= threshold && isNewDay == true) { // only water if the moisture is under 25% and already a new day is, else the plants would be over watered
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

/*
 * This function allows to manually water the plants though a button press
 */
void manualWatering() {
  // get current state of button to avoid conflicts
  manuell_btn_state = digitalRead(manuell_PUMP);

  // check if button is pressed or being pressed
  if (manuell_btn_state == HIGH && lastManuellButtonState == LOW) {
    manuell_btn_pressTime = millis();  // save time how long the user pressed the button to know how long the pump has to be active
  }

  // check if button is released
  if (manuell_btn_state == LOW && lastManuellButtonState == HIGH) {
    // check if the button is being pressed more then 500ms and not just pressed unintentional
    if (millis() - manuell_btn_pressTime >= 500) {
      digitalWrite(pumpPIN, HIGH); //start watering
      Serial.println("Pump starts watering!");
      currentPumpState = true;
    }
  }

  // refresh last state of button
  lastManuellButtonState = manuell_btn_state;

  // check if watering ready to stop
  if (manuell_btn_state == LOW) {
    Serial.println("Pump finished watering!");
    digitalWrite(pumpPIN, LOW);
    currentPumpState = false;
  }
}

void setup() {

  // Start preferences and set a standard value for the watering time to 10 seconds
  preferences.begin("plantid-pref", false); // name preferences to identify later when changing watering time manually by user
  wateringTime = preferences.getInt("wateringTime", 10000);

  // Serial port for debugging
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // set wifi mode

  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Initialize mDNS so the user do not have to enter the web gui by the ip but instead with "plantid.local"
  if (!MDNS.begin("plantid")) {   // Set the hostname to "plantid.local"
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Print ESP32 Local IP Address. only necassary if user wants to enter web gui by ip
  Serial.print("Local IP of ESP32: ");
  Serial.println(WiFi.localIP());

  // Initialize SPIFFS for saving the files into it
  SPIFFS.begin();

  // set sensors as input and pump as output. setting them as input if they are sensors and just getting data and as output if they execute any action
  pinMode(moisturePIN, INPUT);
  pinMode(temperaturePIN, INPUT);
  pinMode(pumpPIN, OUTPUT);
  pinMode(manuell_PUMP, INPUT);

  // Route for root / web page. This page will standard be opended when user opens web gui
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });

  // Route for temperature value. Temperature value will be sent to be displayed on the web gui
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readTemp().c_str());
  });

  // Route for moisture value. Moisture value will be sent to be displayed on the web gui
  server.on("/moisture", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readMoisture().c_str());
  });

  // Route for pump state. Pump State value will be sent to be displayed on the web gui
  server.on("/pumpState", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", readPumpState().c_str());
  });

  // Route to get the current watering time, which is set up
  server.on("/getWateringTime", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"wateringTime\":" + String(wateringTime) + "}";
    request->send(200, "application/json", json);
  });

  // Route to update the watering time, which got set by the user in the web gui
  server.on("/updateWateringTime", HTTP_POST, [](AsyncWebServerRequest *request){
    if(request->hasParam("wateringTime", true)){
      uint32_t newWateringTime = request->getParam("wateringTime", true)->value().toInt();
      wateringTime = newWateringTime;

      //save it to preferences
      preferences.putInt("wateringTime", wateringTime);
      
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Bad Request"); // for error detection if new watering time could be updated
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

  /*
   * Execute all functions in a loop 
   */
  readMoisture();
  getCurrentDay();
  startPump();
  readPumpState();
  manualWatering();
}
