#include "ESP8266WiFi.h"

// WiFi parameters to be configured
const char* ssid = "WIFI-INTERNET"; // Write here your router's username
const char* password = "solares3618"; // Write here your router's passward
WiFiServer server(3000);

void setup(void)
{ 
  Serial.begin(9600);
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
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("192.168.1.107", 8081)) { // Cambia la dirección y el puerto según tu servidor
      Serial.println("Conexión establecida");
      client.println("GET /ruta HTTP/1.1");
      client.println("Host: servidor.com");
      client.println("Connection: close");
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

  WiFiClient client = server.available();
  if (!client) {    return;  }
  while(!client.available()) {
    delay(10);
  }
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  delay(5000); 
}

void sendTcp(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("192.168.1.107", 8081)) { // Cambia la dirección y el puerto según tu servidor
      Serial.println("Conexión establecida");      
      client.write(message);
    } else {
      Serial.println("Fallo en la conexión");
    }
    // Cierra la conexión
    client.stop();
  }
}
