// --- Pines y librerías ---
#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <PID_v1.h>

#define DHT_PIN 13
#define LED_PIN 26
#define VENTILADOR_LED_PIN 27
#define BUZZER_PIN 25

#define BUZZER_CH 0   // Canal LEDC para buzzer
#define FAN_CH 1      // Canal LEDC para ventilador
#define LED_CH 2      // Canal LEDC para LED
#define PWM_FREQ 5000 // Frecuencia PWM ventilador/LED
#define PWM_RES 8     // Resolución PWM 0-255
#define TONE_FREQ 1000 // Frecuencia buzzer

// --- Sensor ---
DHT dht(DHT_PIN, DHT11);

// --- LCD ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- PID ---
double tempActual = 0;
double setpoint = 25.0;
double output = 0;
double Kp = 25.0;
double Ki = 0.2;
double Kd = 10.0;
PID controlPID(&tempActual, &output, &setpoint, Kp, Ki, Kd, REVERSE);

// --- WiFi AP ---
const char* apSSID = "ESP32_PID";
const char* apPassword = "12345678";
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// --- Web ---
WebServer server(80);

// --- Buzzer temporizador ---
unsigned long previousMillis = 0;
bool buzzerState = false;

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
  if (server.hasArg("sp")) setpoint = server.arg("sp").toFloat();
  handleRoot();
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  dht.begin();
  controlPID.SetMode(AUTOMATIC);
  controlPID.SetOutputLimits(0, 255);

  // --- Configurar PWM ---
  ledcSetup(FAN_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(VENTILADOR_LED_PIN, FAN_CH);

  ledcSetup(LED_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(LED_PIN, LED_CH);

  ledcSetup(BUZZER_CH, TONE_FREQ, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CH);

  // --- Configurar AP ---
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID, apPassword);
  Serial.println("Red AP creada!");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  // --- Servidor web ---
  server.on("/", handleRoot);
  server.on("/set", handleSetpoint);
  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();

  tempActual = dht.readTemperature();
  if (isnan(tempActual)) return;

  controlPID.Compute(); // salida normal


  // PWM ventilador y LED
  int pwmValue = constrain((int)output, 0, 255);
  ledcWrite(FAN_CH, pwmValue);
  ledcWrite(LED_CH, pwmValue); // LED simula la velocidad del ventilador

  int porcentaje = map(pwmValue, 0, 255, 0, 100);

  // --- Control buzzer intermitente ---
  unsigned long currentMillis = millis();
  if (tempActual >= setpoint + 2) {
    if (currentMillis - previousMillis >= 1000) { // alternar cada 1 segundo
      buzzerState = !buzzerState;
      previousMillis = currentMillis;
    }
    ledcWrite(BUZZER_CH, buzzerState ? 255 : 0);
  } else if (tempActual > setpoint) {
    ledcWrite(BUZZER_CH, 0); // buzzer apagado
  } else {
    ledcWrite(BUZZER_CH, 0); // buzzer apagado
  }

  // --- LCD ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(tempActual, 1);
  lcd.print("C SP:"); lcd.print(setpoint, 0);

  lcd.setCursor(0, 1);
  lcd.print("Fan:"); lcd.print(porcentaje); lcd.print("%");

  // --- Serial debug ---
  Serial.print("Temp: "); Serial.print(tempActual);
  Serial.print(" | SP: "); Serial.print(setpoint);
  Serial.print(" | PWM: "); Serial.print(pwmValue);
  Serial.print(" | %: "); Serial.println(porcentaje);

  delay(100); // loop rápido para control de buzzer
}
