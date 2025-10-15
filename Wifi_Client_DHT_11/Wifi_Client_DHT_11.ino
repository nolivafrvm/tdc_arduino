// Incluimos librería
#include <DHT.h>
 
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
#define RELAY 0
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

#include "ESP8266WiFi.h"

// WiFi parameters to be configured
const char* ssid = "WifiInternet"; // Write here your router's username
const char* password = "Nico12345"; // Write here your router's passward
WiFiServer server(3000);

void setup(void)
{ 
  Serial.begin(9600);
  // Comenzamos el sensor DHT
  dht.begin();

  //WriteOutput Relay
  pinMode(RELAY, OUTPUT);  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());

  Serial.println("Server ON");
  // Inicializando Server
  server.begin();

}
void loop() {
  //sendTcp("HolaMundo");
  //recievedTcp();


  // Esperamos 5 segundos entre medidas
  delay(3000);
  digitalWrite(RELAY, LOW);
  Serial.println("Relay Low");
  delay(3000);
  Serial.println("Relay High");
  digitalWrite(RELAY, HIGH);
 
  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
  // Leemos la temperatura en grados Fahrenheit
  float f = dht.readTemperature(true);
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
 
  // Calcular el índice de calor en Fahrenheit
  float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);
 
  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  
}

void recievedTcp() {
  WiFiClient client = server.available();
  if (!client) {    return;  }
  while(!client.available()) {
    delay(10);
  }
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  delay(2000); 
}

void sendTcp(const char* message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("192.168.1.107", 12345)) { // Cambia la dirección y el puerto según tu servidor
      Serial.print("Enviando:");      
      Serial.println(message);
      client.write(message);
    } else {
      Serial.println("Fallo en la conexión");
    }
    // Cierra la conexión
    client.stop();
  }
  delay(2000); 
}
