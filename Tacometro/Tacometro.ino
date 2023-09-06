// ---Teoría de control---

// ---Tacómetro---

#define FAN_PULSE 5   //tachymeter fan pin // GPIO5

int rpm;
int count = 0;

unsigned long start_time;


void setup(void)
{ 
  Serial.begin(9600);

  pinMode(FAN_PULSE, INPUT);
    
  attachInterrupt(digitalPinToInterrupt(FAN_PULSE), counter, RISING);

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
  
}

ICACHE_RAM_ATTR void counter() {
  count++;
}
