#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <PID_v1.h>

// ---------- Pines ----------
#define DHT_PIN 13
#define LED_PIN 26
#define VENTILADOR_LED_PIN 27
#define BUZZER_PIN 25


// ---------- Sensor ----------
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Control PID ----------
double tempActual = 0;
double setpoint = 25.0;   // Temperatura deseada
double output = 0;        // Salida PID → PWM ventilador
double Kp = 40.0, Ki = 2.0, Kd = 10.0;

PID controlPID(&tempActual, &output, &setpoint, Kp, Ki, Kd, REVERSE);

// ---------- WiFi ----------

// --- Parámetros AP ---
const char* apSSID = "ESP32_PID";
const char* apPassword = "12345678";  // mínimo 8 caracteres

IPAddress local_IP(192, 168, 4, 1);   // IP fija que quieres
IPAddress gateway(192, 168, 4, 1);    // gateway, normalmente igual que la IP
IPAddress subnet(255, 255, 255, 0);   // máscara de subred

WebServer server(80);

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Control PID - ESP32</title>";
  html += "<style>body{font-family:Arial;text-align:center;background:#f2f2f2;}h1{color:#333;}div{background:white;padding:15px;border-radius:10px;display:inline-block;}</style></head><body>";
  html += "<h1>Monitor PID - Control de Temperatura</h1><div>";
  html += "<p><b>Temperatura actual:</b> " + String(tempActual, 1) + " °C</p>";
  html += "<p><b>Setpoint:</b> " + String(setpoint, 1) + " °C</p>";
  html += "<p><b>Salida PWM:</b> " + String((int)output) + " (" + String(map(output, 0, 255, 0, 100)) + "%)</p>";
  html += "<form action='/set' method='get'>";
  html += "<input type='number' name='sp' step='0.5' value='" + String(setpoint, 1) + "'> °C ";
  html += "<input type='submit' value='Actualizar'>";
  html += "</form></div></body></html>";
  server.send(200, "text/html", html);
}

void handleSetpoint() {
  if (server.hasArg("sp")) {
    setpoint = server.arg("sp").toFloat();
  }
  handleRoot();
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  pinMode(LED_PIN, OUTPUT);
  pinMode(VENTILADOR_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  dht.begin();
  controlPID.SetMode(AUTOMATIC);
  controlPID.SetOutputLimits(0, 255);

  // Conexión WiFi
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);

  Serial.println("Red AP creada!");
  Serial.print("IP fija del AP: ");
  Serial.println(WiFi.softAPIP());

  // Servidor web
  server.on("/", handleRoot);
  server.on("/set", handleSetpoint);
  server.begin();
  Serial.println("Servidor web iniciado");

  // Servidor web
  server.on("/", handleRoot);
  server.on("/set", handleSetpoint);
  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();

  tempActual = dht.readTemperature();

  if (isnan(tempActual)) {
    Serial.println("Fallo en lectura DHT22");
    return;
  }

  controlPID.Compute();

  int pwmValue = (int)output;
  analogWrite(VENTILADOR_LED_PIN, pwmValue);
  int porcentaje = map(pwmValue, 0, 255, 0, 100);

  // Indicadores visuales
  if (tempActual >= setpoint + 3) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  } else if (tempActual > setpoint) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Mostrar en LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(tempActual, 1);
  lcd.print("C SP:"); lcd.print(setpoint, 0);

  lcd.setCursor(0, 1);
  lcd.print("Fan:"); lcd.print(porcentaje); lcd.print("%");

  // Serial para debug
  Serial.print("Temp: "); Serial.print(tempActual);
  Serial.print(" | SP: "); Serial.print(setpoint);
  Serial.print(" | PWM: "); Serial.print(pwmValue);
  Serial.print(" | %: "); Serial.println(porcentaje);

  delay(1000);
}
