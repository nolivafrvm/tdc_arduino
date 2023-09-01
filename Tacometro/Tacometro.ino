// Incluimos librería
 
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
#define RELAY 0

#define FAN_PULSE 5   //tachymeter fan pin // GPIO5
#define FAN_CONTROL 4 //pwn fan pin // GPIO 4
int rpm;
int count = 0;

unsigned long start_time;


void setup(void)
{ 
  Serial.begin(9600);
  // Comenzamos el sensor DHT
  pinMode(FAN_CONTROL, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FAN_PULSE), counter, RISING);
  analogWriteFreq(25000);


}
void loop() {
  start_time = millis();
  count = 0;

  while ((millis() - start_time) < 1000)
  {
    // 1 second pause so count increments
  }

  rpm = (count / 2) * 60;

  Serial.print("The fan speed is ");
  Serial.print(rpm);
  Serial.print(" rpm - count: ");
  Serial.println(count);

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

ICACHE_RAM_ATTR void counter() {
  count++;
}
