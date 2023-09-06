// ---Teoría de control---

// ---Implementación de un PID Fan controller---

// Incluimos librerias
#include <DHT.h>
#include <PID_v2.h>
#include "ESP8266WiFi.h"
 
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2

// Definimos pines del fan cooler
#define FAN_PULSE 5   //tachymeter fan pin // GPIO5
#define FAN_CONTROL 4 //pwn fan pin // GPIO 4

// Definición tipo de sensor
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// PID Variables
double setpoint = 17.70;

double output;

double tempActual;
double humedad;
double fahrenheit; 

double Kp = 60.0, Ki = 1.0, Kd = 1.0;
PID myPID(&tempActual, &output, &setpoint, Kp, Ki, Kd, REVERSE);

// Tacometro
int rpm;
int count = 0; 
unsigned long start_time; 

float porcentaje;

// Periodo grabacion
unsigned long period_record = 10000; 

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
 // Inicializar el PID
  myPID.SetMode(AUTOMATIC);

  // Establecer los límites de salida del calentador (0 a 255 para PWM)
  myPID.SetOutputLimits(0, 255);

  period_record = millis();
}
void loop() {  
  
  obtenerTemperatura();
  
  getRPM();

  controlPid();
  
  mostrarSerial();
}

void controlPid() {
  // Calcular la salida del PID
  myPID.Compute();
  
  // PID Function      
  porcentaje = (output/255) * 100;  

  // Calcula la velocidad del ventilador en función de la salida del PID
  int fanSpeed = map(output, 0, 255, 0, 255); // Ajusta la salida del PID al rango del ventilador

  // Controla la velocidad del ventilador (esto es un ejemplo, podría variar según tu hardware)
  analogWrite(FAN_CONTROL, fanSpeed); // FAN_PIN debe ser reemplazado por el pin adecuado  
}

ICACHE_RAM_ATTR void counter() {
  count++;
}

void obtenerTemperatura() {
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

}

void getRPM() {
  start_time = millis();
  count = 0;

  while ((millis() - start_time) < 1000)
  {
    // 1 seg. para contar vueltas
  }    
   rpm = (count / 2) * 60;   
}

void mostrarSerial() {

  Serial.print("Temperatura: ");  
  Serial.print(tempActual);
  Serial.print(" *C ");  

  Serial.print("| SetPoint: ");
  Serial.print(setpoint);

  Serial.print("| Vel. Ventilador:");
  Serial.print(" | ");
  Serial.print(rpm);
  Serial.print(" | RPM ");

  Serial.print("| Salida del PID: ");
  Serial.print(output);

  Serial.print(" | Potencia Máxima: " );
  Serial.print(porcentaje);
  Serial.println(" %");

}
