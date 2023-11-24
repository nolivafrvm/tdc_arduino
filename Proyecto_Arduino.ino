#include "ESP8266WiFi.h"

// WiFi parameters to be configured
const char *ssid = "Fibertel 2.4";
const char *password = "Salera2812";

const int laserPin = 16;
const int ledPin = 5;
const int relayPin = 4;
const int ldrPin = A0;

int ldrValue;
int lastBrightness = 255;  
boolean lastLDRState = false;  
unsigned long ldrStartTime = 0;  

boolean laserActivated = false;
boolean relayActivated = false;


WiFiServer server(3000);

void setup(void) {
  pinMode(laserPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);


  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  Serial.println("Server ON");
  server.begin();

  fadeInLED();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("192.168.0.83", 8081)) {
      digitalWrite(laserPin, HIGH);
      ldrValue = analogRead(ldrPin);

      Serial.print("LDR Value: ");
      Serial.println(ldrValue);

      boolean currentLDRState = (ldrValue > 350);

      if (currentLDRState != lastLDRState) {
        lastLDRState = currentLDRState;

        if (currentLDRState) {
          if (!laserActivated) {
            ldrStartTime = millis();
            fadeOutLED();

            digitalWrite(relayPin, LOW);
            

            String mensaje = ("Encendido - Enciende LED");
            const char *mensajeChar = mensaje.c_str();

            if (WiFi.status() == WL_CONNECTED) {
              WiFiClient client;
              if (client.connect("192.168.0.83", 3000)) {
                client.write(mensajeChar);
              }
            }
            
            laserActivated = true;
          }
        } else {
          laserActivated = false;
          fadeInLED();
          digitalWrite(relayPin, HIGH);
        }
      }

      if (currentLDRState && (millis() - ldrStartTime < 5000) && (lastBrightness > 127)) {
        ldrStartTime = millis();
      }

      delay(1000);
    }

    // Espera y lee la respuesta del servidor
        String response;
        while (client.available()) {
          char c = client.read();
          response += c  ;
        }

        Serial.println("Response from server: " + response);
  }

  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(10);
  }
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
}

void fadeOutLED() {
  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;
  int startBrightness = lastBrightness;

  while (elapsedTime < 5000 && lastLDRState) {  
    int brightness = map(elapsedTime, 0, 5000, startBrightness, 0);
    analogWrite(ledPin, brightness);
    delay(10);
    elapsedTime = millis() - startTime;
  }

  if (lastLDRState && (millis() - ldrStartTime >= 2000)) {
    int halfBrightness = startBrightness / 2;
    analogWrite(ledPin, halfBrightness);
    lastBrightness = halfBrightness;
  }

  if (lastLDRState && (millis() - ldrStartTime >= 4000)) {
    analogWrite(ledPin, 0);
    lastBrightness = 0;
  }
}

void fadeInLED() {
  for (int brightness = 0; brightness <= 255; brightness += 5) {
    analogWrite(ledPin, brightness);
    delay(20);
  }
  lastBrightness = 255;
}
