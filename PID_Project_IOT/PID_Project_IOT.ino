// ---Teoría de control---

// ---Implementación de un PID Fan controller---



// Incluimos librería
#include <DHT.h>
#include <PID_v2.h>
#include "ESP8266WiFi.h"
 
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
#define RELAY 0
// Definimos pines del fan cooler
#define FAN_PULSE 5   //tachymeter fan pin // GPIO5
#define FAN_CONTROL 4 //pwn fan pin // GPIO 4
// Definición tipo de sensor
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// PID Variables
double setpoint = 17.0;

double output;

double tempActual;
double humedad;
double fahrenheit; 

double Kp = 2, Ki = 5, Kd = 1;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

int rpm;
int count = 0;

// WiFi parameters to be configured
const char* ssid = "WIFI-INTERNET"; // Write here your router's username
const char* password = "solares3618"; // Write here your router's passward
WiFiServer server(3000);

void setup(void)
{ 
  Serial.begin(9600);
  // Comenzamos el sensor DHT
  dht.begin();

  // Fan pin modes
  pinMode(FAN_PULSE, INPUT);  
  pinMode(FAN_CONTROL, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FAN_PULSE), counter, RISING);

  //WriteOutput Relay
  pinMode(RELAY, OUTPUT);  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  
  Serial.println(WiFi.localIP());

  Serial.println("Server ON");
  // Inicializando Server
  server.begin();

    myPID.Start(tempActual,  // input
              0,                      // current output
              25);                   // setpoint
}
void loop() {
  //sendTcp("HolaMundo");
  //recievedTcp();
  delay(1000);

  rpm = count * 60 / 2;
  count = 0;
  Serial.print("RPM: ");
  Serial.println(rpm);

  // Esperamos 5 segundos entre medidas
  //delay(3000);
  digitalWrite(RELAY, HIGH);
  Serial.println("Relay Low");
  //delay(3000);
  //Serial.println("Relay High");
  //digitalWrite(RELAY, HIGH);
 
  // Leemos la humedad relativa
  humedad = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  tempActual = dht.readTemperature();
  // Leemos la temperatura en grados Fahrenheit
  fahrenheit = dht.readTemperature(true);
 
  // Comprobamos si ha habido algún error en la lectura
  if (isnan(humedad) || isnan(tempActual) || isnan(fahrenheit)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }
 
  // Calcular el índice de calor en Fahrenheit
  float hif = dht.computeHeatIndex(fahrenheit, humedad);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(tempActual, humedad, false);
 
  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(tempActual);
  Serial.print(" *C ");
  Serial.print(fahrenheit);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  // PID Function

  //myPID.Compute();
  //output = myPID.Run(tempActual);
  output = myPID.Run(tempActual);
  Serial.print("output: ");
  Serial.println(output);

  // Calcula la velocidad del ventilador en función de la salida del PID
  int fanSpeed = map(output, 0, 255, 0, 255); // Ajusta la salida del PID al rango del ventilador
  Serial.print("fanSpeed: ");
  Serial.println(fanSpeed);

  // Controla la velocidad del ventilador (esto es un ejemplo, podría variar según tu hardware)
  analogWrite(FAN_CONTROL, fanSpeed); // FAN_PIN debe ser reemplazado por el pin adecuado

  
}

ICACHE_RAM_ATTR void counter() {
  count++;
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
