// Definición de pines
const int sensorPin = 5;   // Pin conectado al sensor de inclinación
const int relayPin = 4;    // Pin conectado al relé
const int relayOutput = 2; // Pin conectado al pin de salida del relé (NO o NC)
int estadoAlerta = 0;
long tiempo = 0;
long tiempoCero = 0;
unsigned long millisPrevio = 0;
 
void setup() {
  pinMode(sensorPin, INPUT);   // Configurar el pin del sensor como entrada
  pinMode(relayPin, OUTPUT);   // Configurar el pin de control del relé como salida
  pinMode(relayOutput, OUTPUT); // Configurar el pin de salida del relé como salida
  tiempoCero = millis();
  Serial.println(tiempoCero);
  Serial.begin(9600);
}
 
//esta es la funcion que define la velocidad del parpadeo segun el tiempo que se ingrese como parametro  
void parpadeo (int tiempoDelay){
  digitalWrite(relayOutput,LOW);
  delay(tiempoDelay);
  digitalWrite(relayOutput,HIGH);
}


void loop() {
  delay(100);
  int sensorValue = digitalRead(sensorPin); // Leer el estado del sensor
  Serial.println(sensorValue);
  Serial.println(tiempoCero);
  Serial.println("tiempo vale");
  Serial.println(tiempo);
  // Si el sensor de inclinación está activado (cambia según tu tipo de sensor)

  // el sensor nuestro "arranca" en HIGH 

  if (sensorValue == LOW) {
      //Serial.print("el coso se dio vuelta");
      digitalWrite(relayPin, HIGH);  // Activar el relé
      digitalWrite(relayOutput, HIGH); // Activar el pin de salida del relé (NO o NC)
      //se arranca a contar el tiempo, restandole el tiempo previo para resetear el contador 
      tiempo = millis() - millisPrevio;


    if ((tiempo > (tiempoCero + 10000)) && (tiempo < (tiempoCero + 15000))){
      Serial.println("alerta 10 segundos");
      parpadeo(1000);
    } else {
      if ((tiempo > tiempoCero + 15000) && (tiempo < tiempoCero + 20000)){
        Serial.println("alerta 15 segundos");
        parpadeo(500);
      } else {
        if (tiempo > tiempoCero + 20000){
          Serial.println("alerta 20 segundos");
          parpadeo(250);
        }
      } 
    
    
  // cuando se da vuelta, envia la señal LOW
  }} else {
    //Serial.print("se dio vuelta");
    millisPrevio = millis();
    
    digitalWrite(relayPin, LOW);   // Desactivar el relé
    digitalWrite(relayOutput, LOW);  // Desactivar el pin de salida del relé (NO o NC)
    
  }
}