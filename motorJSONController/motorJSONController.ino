#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

// WiFi Credentials
const char *ssid = "Redmi";
const char *password = "12345670";

// Define the pin connections
#define PWMB D6
#define BIN2 D5
#define BIN1 D4
#define STBY D3
#define AIN2 D2
#define AIN1 D0
#define PWMA D1

void setup() {
  // Set the motor control pins as outputs
  pinMode(PWMB, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(PWMA, OUTPUT);

  // Set the WiFi settings
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start web server and web socket
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // Standby pin must be high before motors can be controlled
  digitalWrite(STBY, HIGH);

  webSocket.loop();
  server.handleClient();
  // if (Serial.available() > 0) {
  //   char c[] = { (char)Serial.read() };
  //   webSocket.broadcastTXT(c, sizeof(c));
  // }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    // Create a StaticJsonDocument with a capacity of 48 bytes
    StaticJsonDocument<48> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, payload, length);
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    // Extract values
    int x = doc["x"].as<int>();
    int y = doc["y"].as<int>();

    // Control motor A
    if (x >= 2 && x <= 155) {
      // Control motor A
      digitalWrite(AIN1, HIGH);    // One direction
      digitalWrite(AIN2, LOW);     // Other direction
      analogWrite(PWMA, x + 200);  // Speed proportional to x
      Serial.print("x in if: ");
      Serial.println(x);
    } else if (x <= -2 && x >= -155) {
      // Control motor A
      digitalWrite(AIN1, LOW);   // One direction
      digitalWrite(AIN2, HIGH);  // Other direction
      int speed = abs(x) + 200;  // Add 100 to the absolute value of x
      if (speed > 1023) {
        speed = 1023;  // Ensure speed does not exceed 1023
      }

      analogWrite(PWMA, speed);  // Speed proportional to absolute value of x
      Serial.print("x in else if: ");
      Serial.println(x);
    } else {
      digitalWrite(AIN1, LOW);  // One direction
      digitalWrite(AIN2, LOW);  // Other direction
      analogWrite(PWMA, 0);
    }



    // Control motor B
    if (y >= 2 && x <= 155) {
      digitalWrite(BIN1, HIGH);    // One direction
      digitalWrite(BIN2, LOW);     // Other direction
      analogWrite(PWMB, y + 200);  // Full speed
      Serial.print("y: ");
      Serial.println(y);
    } else if (y <= -2 && y >= -155) {
      int speed = abs(y) + 200;
      digitalWrite(BIN1, LOW);   // One direction
      digitalWrite(BIN2, HIGH);  // Other direction
      analogWrite(PWMB, speed);  // Full speed
      Serial.print("y: ");
      Serial.println(y);
    } else {
      digitalWrite(BIN1, LOW);  // One direction
      digitalWrite(BIN2, LOW);  // Other direction
      analogWrite(PWMB, 0);     // Full speed
    }
  }
}
