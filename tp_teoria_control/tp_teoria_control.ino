#include "ESP8266WiFi.h"

// WiFi parameters to be configured
const char* ssid = "Bacci"; // Write here your router's username
const char* password = "bacci123"; // Write here your router's passward
WiFiServer server(3000);

const int s0 = D1;
const int s1 = D2;
const int s2 = D3;
const int s3 = D4;
const int out = D5;
const int relay = D7;
const int led = D8;

int rojo = 0;
int verde = 0;
int azul = 0;

unsigned long t1 = 1000UL;
unsigned long t2 = 2000UL;
unsigned long t3 = 3000UL;
unsigned long t4 = 4000UL;

unsigned long tiempoAnterior = 0;
bool alcanzadoVerde100 = false;
bool detectoVerde = false;

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

  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);
  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);

  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  tiempoAnterior=millis();
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("192.168.43.134", 8081)) { // Cambia la dirección y el puerto según tu servidor
      Serial.println("Conexión establecida");
    } else {
      Serial.println("Fallo en la conexión");
    }

    color();

    // Si se detecto el color verde, se deja de actualizar el tiempo anterior
    if(!detectoVerde){
      tiempoAnterior = millis();
    }

    //Serial.print("Tiempo Anterior: ");
    //Serial.println(tiempoAnterior);

    if (rojo < azul && verde > azul && rojo < 35) {
      alcanzadoVerde100 = false;
      detectoVerde = false;
      client.println("ROJO");
      digitalWrite(relay, LOW);
    } else if (azul < rojo && azul < verde && verde < rojo) {
      alcanzadoVerde100 = false;
      detectoVerde = false;
      client.println("AZUL");
      digitalWrite(relay, LOW);
    } else if (rojo > verde && azul > verde) {
      detectoVerde = true;
      // Incrementa la intensidad del LED con el tiempo

      unsigned long tiempoActual = millis();
      
      if ((tiempoActual - tiempoAnterior) >= t1 && (tiempoActual - tiempoAnterior) <= t2 && !alcanzadoVerde100) {
          client.println("VERDE %25");
          analogWrite(led, 64);

      } else if ((tiempoActual - tiempoAnterior) > t2 && (tiempoActual - tiempoAnterior) <= t3 && !alcanzadoVerde100) {
          client.println("VERDE %50");
          analogWrite(led, 128);
      } else if ((tiempoActual - tiempoAnterior) > t3 && (tiempoActual - tiempoAnterior) <= t4 && !alcanzadoVerde100) {
          client.println("VERDE %75");
          analogWrite(led, 192);
      } else if ((tiempoActual - tiempoAnterior) > t4 && !alcanzadoVerde100) {
          alcanzadoVerde100 = true;
          client.println("VERDE %100");
          analogWrite(led, 254);
          // Reinicia el tiempoAnterior para comenzar el siguiente ciclo
          tiempoAnterior = tiempoActual;
      }

      digitalWrite(relay, HIGH);
    } else {
      alcanzadoVerde100 = false;
      detectoVerde = false;
      digitalWrite(relay, LOW);
    }
    
    // Espera y lee la respuesta del servidor
    while (client.available()) {
      Serial.println("Client Available");
      char c = client.read();
      Serial.print(c);
    }

    delay(900);
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

void color() {
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  rojo = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s3, HIGH);
  azul = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s2, HIGH);
  verde = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
}
