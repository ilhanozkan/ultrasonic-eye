#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

// Buzzer pin tanimlamasi:
#define buzzer 13

// Initialize sensor parameters
float distance = 0.0;

// Sesli ikaz icin mesafe sabiti:
float intruderDistance = 50.0;

// WiFi agi bilgileri:
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Declare websocket client class variable
WebSocketsClient webSocket;

// Allocate the JSON document
StaticJsonDocument<200> doc;

void setup() {
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  
  // Connect to WiFi
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  // server address, port, and URL path
  webSocket.begin("192.168.4.1", 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, payload);
  
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    
    distance = doc["distance"];
    //Serial.println(distance);
    alarm_condition();

    // Send acknowledgement to the client
    webSocket.sendTXT("{\"status\":\"OK\"}");
}

void loop() {
  webSocket.loop();
  //alarm_condition();
}

// Sesli ikaz komutunu kontrollu calistiran fonksiyon:
void alarm_condition() {
  // Sesli ikazin belirtilen mesafeden kucuk oldugunda baslamasini saglayan kontrol blogudur.
  if(distance <= intruderDistance && distance != 0.0){
    digitalWrite(buzzer, HIGH);
    Serial.println(distance);
  }
  else {
    digitalWrite(buzzer, LOW);
    Serial.println("degil");
  }
}
