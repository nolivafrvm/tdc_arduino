// librerias
#include "ESP8266WiFi.h"
#include <Wire.h>
#include <RTClib.h>

// Declaraciones de pines
int sensorAnalogico = A0; 
int Led_Pin = 12;  
int sensorRele = 2;      

// Declaracion wifi
const char* ssid = "Sancheta";
const char* password = "20sanche02";
WiFiServer server(3000);
RTC_DS3231 rtc;

void setup ()
{
  // Modos de los pines
  pinMode (sensorAnalogico, INPUT);
  pinMode(Led_Pin, OUTPUT);
  pinMode(sensorRele, OUTPUT);

  Serial.begin (9600); 

  // Rele y pin activados
  digitalWrite(Led_Pin, HIGH);  
  digitalWrite(sensorRele, HIGH);  

  // Conexion WiFi
  WiFi.begin(ssid, password);
  

  // imprime hasta que conecta
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado | ip: ");
  Serial.println(WiFi.localIP());
  Serial.println("Server ON");

}

void loop ()
{
  
  float tempAnalogica;

  tempAnalogica = analogRead(sensorAnalogico);
  float tempF = 125.315-0.175529*tempAnalogica-2.5;
  float tempCelcius = ((tempF-32)/1.8)-5;

 Serial.print ("Valor temperatura: "); Serial.print (tempCelcius, 4); Serial.print (" C");
 if (tempCelcius<28.0 ){
    analogWrite(Led_Pin, 0);
    Serial.println ("  Nivel PWM 1 ");
    sendTcp("Nivel PWM 1 - apagado - " + String(tempCelcius)+" C");

 } else if (tempCelcius>=28.0 && tempCelcius<29.0 ){
    analogWrite(Led_Pin, 50);
    Serial.println ("  Nivel PWM 2  ");

    sendTcp("Nivel PWM 2 - " + String(tempCelcius)+" C");

 }  else if (tempCelcius>=29.0 && tempCelcius<30.0 ){
    analogWrite(Led_Pin, 150);
    Serial.println ("  Nivel PWM 3  ");

    sendTcp("Nivel PWM 3 - " + String(tempCelcius)+" C");

 } else if (tempCelcius>=30.0){
    analogWrite(Led_Pin, 255);
    Serial.println ( "  Nivel PWM 4  ");

    sendTcp("Nivel PWM 4 - maximo - " + String(tempCelcius)+" C");

 }

 Serial.println ("----------------------------------------------------------------");
 delay (2500);
 
}


void sendTcp(String message) {

if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    // Trata de mandar el mensaje
    // ip y puerto
    if (client.connect("192.168.19.3", 8080)) { 
      Serial.println("Conexión establecida");
      client.println(message);
      client.println();
      client.write("HolaMundo");
    } else {
      Serial.println("Fallo en la conexión");
    }

    // Espera y lee la respuesta del servidor
    while (client.available()) {
      Serial.println("Client Available");
      char c = client.read();
      Serial.print(c);
    }

    // Cierra la conexión
    client.stop();
  }

}