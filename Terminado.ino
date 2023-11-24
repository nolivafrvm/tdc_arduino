//LIBRERIAS
#include <DHT.h>
#include "ESP8266WiFi.h"

// CONFIGURACION DE PARAMETROS WIFI
const char *ssid = "w-ala2-A"; // NOMBRE DE LA RED
const char *password = "ala2frvm"; // CONTRASEÑA DE LA RED

#define DHTPIN 12    // PIN DEL SENSOR
#define DHTTYPE DHT11 // SENSOR UTILIZADO
#define RELAY_PIN 5  // PIN DEL RELE
#define PWM_LED 0 // PIN DE ALIMENTACION DEL LED

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(RELAY_PIN, OUTPUT); // CONFIGURAR PIN DE RELE COMO SALIDA
  pinMode(PWM_LED, OUTPUT); // CONDIFURAR PIN PWM COMO SALIDA

  digitalWrite(PWM_LED, HIGH);  //SETEAR PWM EN HIGH
  digitalWrite(RELAY_PIN, HIGH); //SETEAR EL RELE EN HIGH
  
  WiFi.mode(WIFI_STA);  //TIPO DE CONEXION
  WiFi.begin(ssid, password); //ESTABLECER CONEXION WIFI
  

  while (WiFi.status() != WL_CONNECTED) { //CONECTANDO...
     delay(700);
     Serial.print(".");
  }

  //CUANDO SE ESTABLECE LA CONEXION
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());  //MOSTRAR IP PLACA
}

void loop() {
  delay(2000); 

  //VARIABLES DE LECTURA
  float humedad = dht.readHumidity();   
  float temperatura = dht.readTemperature();

  //MOSTRAR DATOS EN MONITOR
  if (isnan(humedad) || isnan(temperatura)) {
    Serial.println("Error al leer el sensor DHT");
  } else {
    Serial.print("Humedad: ");
    Serial.print(humedad);
    Serial.print("%\t");
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println("°C");


  //VALOR DE INTENSIDAD DE LED
    int intensidadLed = 0;

  //VALORES DE INTENSIDAD EN FUNCION DE LA TEMPERATURA
    if (temperatura < 29.0) {
      intensidadLed = 0; // Apaga el LED
      sendTcp("T:" + String(temperatura) + "Led Apagado");
    } 
    //SI LA TEMPERATURA ES SUPERIOR A 29°
    else if (temperatura >= 29.0 && temperatura < 30.0) {
      intensidadLed = 64; // 25% de intensidad
      sendTcp("T:" + String(temperatura) + "Led Encendido 25%");
    }
    //SI LA TEMPERATURA ES SUPERIOR A 30° Y MENOR A 31° 
    else if (temperatura >= 30.0 && temperatura < 31.0) {
      intensidadLed = 128; // 50% de intensidad
      sendTcp("T:" + String(temperatura) + "Led Encendido 50%");
    } 
    //SI LA TEMPERATURA ES SUPERIOR A 31° Y MENOR A 32°
    else if (temperatura >= 31.0 && temperatura < 32.0) {
      intensidadLed = 191; // 75% de intensidad
      sendTcp("T:" + String(temperatura) + "Led Encendido 75%");
    } 
    //SI LA TEMPERATURA ES SUPERIOR A 32°
    else {
      intensidadLed = 255; // 100% de intensidad
      sendTcp("T:" + String(temperatura) + "Led Encendido 100%");
    }

  //SETEAR EL NIVEL DE INTENSIDAD EN EL LED CON PWM
    analogWrite(PWM_LED, intensidadLed);
    delay(1000);

  }
}

//ENVIO DE MENSAJES TCP
void sendTcp(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    //SETEAMOS DIRECCION IP Y PUERTO
    if (client.connect("192.168.201.247", 8081)) {
      //SI LA CONEXION SE ESTABLECE SE ENVIA EL MENSAJE  
      client.print(message);
    } else {
      Serial.println("Fallo en la conexión");
    }
    //CIERRA LA CONEXION
    client.stop();
  }
}


