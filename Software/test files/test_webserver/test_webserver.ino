#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>

AsyncWebServer server(80);

const char* ssid = "LifanHome";
const char* password = "nexapote2620!";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found"); 
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.print("Local IP of ESP32: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "It worked, congrats!");
  });

  server.onNotFound(notFound);
}

void loop() {
}
