// Incluimos librería
#include <DHT.h>

 
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
#define RELAY 0
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// PID Variables
double setpoint = 17.0;

double output;

double tempActual;
double humedad;
double fahrenheit; 


#define FAN_PULSE 5   //tachymeter fan pin // GPIO5
#define FAN_CONTROL 4 //pwn fan pin // GPIO 4
int rpm;
int count = 0;


void setup(void)
{ 
  Serial.begin(9600);
  // Comenzamos el sensor DHT
  dht.begin();

  // Fan   
  //pinMode(FAN_PULSE, INPUT);
  //digitalWrite(FAN_PULSE, HIGH);
  //pinMode(FAN_CONTROL, OUTPUT);
//  attachInterrupt(digitalPinToInterrupt(FAN_PULSE), counter, RISING);

  

}
void loop() {

  delay(1000);

  rpm = count * 60 / 2;
  count = 0;
  Serial.print("RPM: ");
  Serial.println(rpm);
 
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
    

  analogWriteRange(25);
  // Calcula la velocidad del ventilador en función de la salida del PID
  for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    analogWrite(FAN_CONTROL, dutyCycle);
    delay(10); // Espera 10 milisegundos antes de cambiar la velocidad
  }
  Serial.println("Maxima Velocidad");
  delay(10000); // Espera 1 segundo a máxima velocidad
  
  for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
    analogWrite(FAN_CONTROL, dutyCycle);
    delay(10); // Espera 10 milisegundos antes de cambiar la velocidad
  }
  Serial.println("Minima Velocidad");
  delay(10000); // Espera 1 segundo a mínima velocidad


  
}


