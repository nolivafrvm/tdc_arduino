// ---Teoría de control---

// ---Implementación de Pulse Width Modulation | modulación por ancho o de pulso---

#define FAN_PULSE 5   //Tacometro // GPIO5
#define FAN_CONTROL 4 //pwn Fan pin // GPIO 4

int rpm;
int count = 0;
unsigned long tiempo;
unsigned long start_time;

void setup(void)
{ 
  Serial.begin(9600);
  // Fan pin configuration   
  pinMode(FAN_PULSE, INPUT);
  pinMode(FAN_CONTROL, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(FAN_PULSE), counter, RISING);
  tiempo = millis();
}
void loop() {
 
  while(millis()-tiempo<5000) {
    for (int dutyCycle = 0; dutyCycle <= 255; dutyCycle++) {
    analogWrite(FAN_CONTROL, dutyCycle);
    delay(100);     
  }  
  }
  getRPM("Velocidad Maxima");
  tiempo = millis();
  
  while(millis()-tiempo < 5000) {
    for (int dutyCycle = 255; dutyCycle >= 0; dutyCycle--) {
    analogWrite(FAN_CONTROL, 0);
    delay(100); // Espera 10 milisegundos antes de cambiar la velocidad
    }    
  } 
  getRPM("Velocidad Minima");
  tiempo = millis();   
  
}


ICACHE_RAM_ATTR void counter() {
  count++;
}

void getRPM(const char* label) {
  start_time = millis();
  count = 0;

  while ((millis() - start_time) < 1000)
  {
    // 1 seg. para contar vueltas
  }    
   rpm = (count / 2) * 60;
   Serial.print(label);
   Serial.print(" - ");
   Serial.print(rpm);
   Serial.println("rpm");
}


