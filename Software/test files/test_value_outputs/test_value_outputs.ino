#define moisturePIN 34
#define temperaturePIN 35
#define pumpPIN 25

bool currentPumpState = false;
float moisture = 0;
float temperature = 0;

String readMoisture() {
  float moistvoltage = (analogRead(moisturePIN)/1024.0);
  Serial.print("analog moist");
  Serial.println(analogRead(moisturePIN));
  moisture = ((moistvoltage*50.0)/3.0); //converting incoming voltage value in %
  String moistureVal = String(moisture);
  Serial.print("float moist: ");
  Serial.println(moisture);
  Serial.print("String moist: ");
  Serial.println(moistureVal);
  if (isnan(moisture)) {
    Serial.println("Could not read any moisture value!");
    return "";
  } else {
    return moistureVal;
  }
}

String readTemp() {
  float tempvoltage = (analogRead(temperaturePIN)/1024.0);
  Serial.print("analog temp");
  Serial.println(analogRead(temperaturePIN));
  temperature = ((tempvoltage-0.5)*100.0); //converting incoming voltage value in °C
  String temperatureVal = String(temperature);
  Serial.print("float temp: ");
  Serial.println(temperature);
  Serial.print("String temp: ");
  Serial.println(temperatureVal);
  if (isnan(temperature)) {
    Serial.println("Could not read any temperature value!");
    return "";
  } else {
    return temperatureVal;
  }
}

String readPumpState() {
  String pumpStr;
  if(currentPumpState = true) {
    pumpStr = "Pump active";
  } else {
    pumpStr = "Pump inactive";
  }
  return pumpStr;
}

void setup() {
  Serial.begin(115200);

  pinMode(moisturePIN, INPUT);
  pinMode(temperaturePIN, INPUT);
  pinMode(pumpPIN, OUTPUT);

  readMoisture();
  readTemp();
  readPumpState();
}

void loop() {
}
