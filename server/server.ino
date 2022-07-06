#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Ticker.h>

// WiFi agi bilgileri:
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Ultrasonic sensor pin tanimlamasi:
#define echo 18
#define trig 5

// Mesafe olcumu ve sureklilik degiskenleri:
float duration;
float distance = 0.0;
float prevDistance = 0.0;

// Instantiate server objects
WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

// Declare/initialize timer variables
Ticker timer;
bool read_data = false;

// Raw string literal for containing the page to be sent to clients
char webpage[] PROGMEM = R"=====(
<html>
<head>
  <script>
    var Socket;
    function init() {
      Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      Socket.onmessage = function(event){
        // receive the color data from the server
        var data = JSON.parse(event.data);
        console.log(data);
      }
    }  
  </script>
</head>
<body onload="javascript:init()">
  <h4>Websocket client served from the Sensor Board!</h4>
</body>
</html>
)=====";

void setup() {
  // Seri iletisimi baslat.
  Serial.begin(115200);
  
  // WiFi agini baslat
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  // define the routes in which the server is accessible
  server.on("/",[](){
    server.send_P(200, "text/html", webpage);  
  });

  // initialize server and websockets
  server.begin();
  webSocket.begin();
  
  // initialize timer function
  timer.attach(/*rate in secs*/ 0.1, /*callback*/ readData);
  
  // handling incoming messages on the websocket
  webSocket.onEvent(webSocketEvent);
}

void readData() {
  // should only be used to set/unset flags
  read_data = true;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if(type == WStype_TEXT)
  {
    // processs any returned data
    Serial.printf("payload [%u]: %s\n", num, payload);
  }
}

void loop() {
  webSocket.loop();
  server.handleClient();  
  if(read_data){
    timeMeasurement();
    distance = (float)duration * (0.0343) / 2;

    if (prevDistance != distance) {
      String json = "{\"distance\":";
      json += distance;
      json += "}";
      prevDistance = distance; 
      
      Serial.println(json); // DEBUGGING
      webSocket.broadcastTXT(json.c_str(), json.length()); 
    }
  }
}

// Mesafe olcumunun zaman araligini belirten fonksiyon:
void timeMeasurement() { // Uzaklik sensorunun mesafe olcum icin gecikme suresini belirt.
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duration = pulseIn(echo, HIGH);
}
