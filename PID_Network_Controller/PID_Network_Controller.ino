// ---Teoría de control---

// ---Implementación de un PID Fan controller---

// Incluimos librerias
#include <DHT.h>
#include <PID_v2.h>
#include "ESP8266WiFi.h"
#include <EEPROM.h>

byte defaultConfig;  // 0 es default // 1 Configurado remotamente

struct ConfigParameters {
  double Kp = 60.0;
  double Kd = 1.0;
  double Ki = 1.0;
  double setpoint = 17.70;
  char serverIP[16] = "192.168.1.107";
  int portServer = 12345;
  char ipaddress[16] = "192.168.1.102";
  int port = 3000;
  int idConfiguration = 1;
};


ConfigParameters configParam;

// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2

// Definimos pines del fan cooler
#define FAN_PULSE 5    //tachymeter fan pin // GPIO5
#define FAN_CONTROL 4  //pwn fan pin // GPIO 4

#define RELAY 0

// Definición tipo de sensor
#define DHTTYPE DHT11

// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

int idDevice = 1;

double output;

double tempActual;
double humedad;
double fahrenheit;

PID myPID(&tempActual, &output, &configParam.setpoint, configParam.Kp, configParam.Ki, configParam.Kd, REVERSE);

// Tacometro
int rpm;
int count = 0;
int fanSpeed = 0;
unsigned long start_time;

float porcentaje;

// Periodo grabacion
unsigned long period_record = 10000;
unsigned long period_last;
unsigned long period_fan_last;
unsigned long period_fan_action = 10000;
// WiFi parameters to be configured
const char* ssid = "WIFI-INTERNET";    // Write here your router's username
const char* password = "solares3618";  // Write here your router's passward
WiFiServer server(configParam.port);

String message;
void setup(void) {
  Serial.begin(9600);

  // Inicializamos la EEPROM
  EEPROM.begin(512);
  setupConfig();

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

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  Serial.println("");
  Serial.print("WiFi connected: ");

  Serial.print(WiFi.localIP());

  Serial.println(" | Server ON");
  // Inicializando Server
  server.begin();
  // Inicializar el PID
  myPID.SetMode(AUTOMATIC);

  // Establecer los límites de salida del calentador (0 a 255 para PWM)
  myPID.SetOutputLimits(0, 255);

  period_last = millis();
}
void loop() {

  obtenerTemperatura();

  getRPM();  

  controlPid();

  controlFan();
  
  mostrarSerial();

  recievedTcp();
  

  if (millis() - period_last > period_record) {
    if (!(isnan(humedad) || isnan(tempActual) || isnan(fahrenheit))) {
      period_last = millis();
      message = buildMessageToSend(1);
      sendTcp(message);
    }
  }
}

void controlPid() {
  // Calcular la salida del PID
  myPID.Compute();

  // PID Function
  porcentaje = (output / 255) * 100;

  // Calcula la velocidad del ventilador en función de la salida del PID
  fanSpeed = map(output, 0, 255, 0, 255);  // Ajusta la salida del PID al rango del ventilador

  // Controla la velocidad del ventilador (esto es un ejemplo, podría variar según tu hardware)
  analogWrite(FAN_CONTROL, fanSpeed);
}

ICACHE_RAM_ATTR void counter() {
  count++;
}

void controlFan() {
  if (fanSpeed > 0) {
    if (millis() - period_fan_last > period_fan_action) {
      period_fan_last = millis();
      digitalWrite(RELAY, HIGH);
      // Controlar la velocidad del cooler, si no funciona mandar un mail
      getRPM();
      if (rpm == 0) {
        // Fan cooler it's not working - send notification
        message = buildMessageToSend(2);
        sendTcp(message);
      }
    }
  } else {
    digitalWrite(RELAY, LOW);
  }
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
    // Error de sensor
    message = buildMessageToSend(3);
    sendTcp(message);
    Serial.println("Error obteniendo los datos del sensor DHT11");
    tempActual = configParam.setpoint;  // Valor para que no sea indefinido el output
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

  while ((millis() - start_time) < 1000) {
    // 1 seg. para contar vueltas
  }
  rpm = (count / 2) * 60;
}

void mostrarSerial() {

  Serial.print("Temperatura: ");
  Serial.print(tempActual);
  Serial.print(" *C ");

  Serial.print("| SetPoint: ");
  Serial.print(configParam.setpoint);

  Serial.print("| Vel. Ventilador:");
  Serial.print(" | ");
  Serial.print(rpm);
  Serial.print(" | RPM ");

  Serial.print("| Salida del PID: ");
  Serial.print(output);

  Serial.print(" | Potencia Máxima: ");
  Serial.print(porcentaje);
  Serial.println(" %");
}

void recievedTcp() {
  WiFiClient client = server.available();
  if (!client) { return; }
  while (!client.available()) {
    delay(10);
  }
  String recievedPacket = client.readStringUntil('\r');
  // Ver el contenido
  // Protocolo de comunicacion -> action;kd;kp;ki;setpoint;ipaddressserver;portServer;ipaddress;port;idConfig
  const int maxTokens = 10;  // Máximo número de tokens esperados
  String tokens[maxTokens];  // Arreglo para almacenar los tokens

  splitString(recievedPacket, ';', tokens, maxTokens);

  int action = tokens[0].toInt();

  switch (action) {
    case 1:
      Serial.print("Guardando Configuración - ");
      saveConfiguration(tokens);
      break;
    case 2:
      Serial.print("Ejecutando Accion Relay ON - ");
      actionRelay(1);
      break;
    case 3:
      Serial.print("Ejecutando Accion Relay OFF - ");
      actionRelay(0);
      break;
    case 4:
      Serial.print("Set Default Configuration - ");
      setDefaultConfiguration();
      break;
    default:
      Serial.println("Acción desconocida");
      break;
  }

  Serial.println(recievedPacket);
  client.flush();

  delay(1000);
}

String buildMessageToSend(int option) {
  switch (option) {
    case 1:
      return "1;" + String(tempActual) + ";" + String(output) + ";" + String(rpm) + ";" + String(configParam.Kp) + ";" + String(configParam.Kd) + ";" + String(configParam.Ki) + ";" + String(configParam.setpoint) + ";" + String(porcentaje) + ";" + String(configParam.idConfiguration) + ";" + String(idDevice);
      break;
    case 2:
      return "2;Error en el ventilador";
      break;
    case 3:
      return "3;Error en el sensor";
      break;
    default:
      Serial.println("Opción desconocida");
      break;
  }
  return "";
}

void sendTcp(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect(configParam.serverIP, configParam.portServer)) {  // Cambia la dirección y el puerto según tu servidor
      Serial.print("Enviando:");
      Serial.println(message);
      client.write(message.c_str());
    } else {
      Serial.println("Fallo en la conexión");
    }
    client.stop();
  }
  delay(200);
}

void setupConfig() {
  Serial.println("Looking for configuration");

  EEPROM.get(0, defaultConfig);
  Serial.print("defaultConfig: ");
  Serial.println(defaultConfig);
  if (defaultConfig != 0) {  // Default values;
    Serial.println("Getting remote configuration setting in EEPROM");
    EEPROM.get(10, configParam);
    Serial.println(configParam.serverIP);
    Serial.println(configParam.portServer);
    Serial.println(configParam.Ki);
    Serial.println(configParam.Kd);
    Serial.println(configParam.Kp);
    Serial.println(configParam.setpoint);
    Serial.println(configParam.ipaddress);
    Serial.println(configParam.port);
  } else {
    Serial.println("Using default config.");
  }
}


void splitString(const String& input, char delimiter, String tokens[], int maxTokens) {
  int tokenIndex = 0;  // Índice actual de tokens
  int startIndex = 0;  // Índice de inicio de cada token

  // Recorre la cadena de entrada
  for (int i = 0; i < input.length(); i++) {
    char currentChar = input.charAt(i);

    // Si encontramos el delimitador o llegamos al final de la cadena
    if (currentChar == delimiter || i == input.length() - 1) {
      // Calcula la longitud del token
      int tokenLength = i - startIndex + 1;

      // Verifica si hemos alcanzado el límite máximo de tokens
      if (tokenIndex < maxTokens) {
        // Extrae el token de la cadena de entrada
        String token = input.substring(startIndex, startIndex + tokenLength);

        // Almacena el token en el arreglo de tokens
        for (int i = 0; i < token.length(); i++) {
          if (token[i] == ';') {
            token.remove(i, 1);
          }
        }

        tokens[tokenIndex] = token;

        // Incrementa el índice de tokens
        tokenIndex++;
      }

      // Actualiza el índice de inicio para el próximo token
      startIndex = i + 1;
    }
  }
}


void saveConfiguration(String tokens[]) {
  // example: 1;2.0;1.0;3.0;16.0;192.168.1.107;12345;192.168.1.100;3000;1
  Serial.println("Saving configuration");
  configParam.Kp = tokens[1].toDouble();
  configParam.Kd = tokens[2].toDouble();
  configParam.Ki = tokens[3].toDouble();
  configParam.setpoint = tokens[4].toDouble();
  tokens[5].toCharArray(configParam.serverIP, 15);
  configParam.portServer = tokens[6].toInt();
  tokens[7].toCharArray(configParam.ipaddress, 15);
  configParam.port = tokens[8].toInt();
  configParam.idConfiguration = tokens[9].toInt();

  // setear default = 1;
  EEPROM.put(0, 1);

  // setear valores
  EEPROM.put(10, configParam);
  EEPROM.commit();
}

void setDefaultConfiguration() {
  // setear default = 0;
  EEPROM.put(0, 0);
  EEPROM.commit();
}

void actionRelay(byte on_off) {
  if (on_off == 1) {
    digitalWrite(RELAY, HIGH);
  } else {
    digitalWrite(RELAY, LOW);
  }
}
