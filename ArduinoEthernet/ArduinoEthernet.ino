#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0xDA, 0xAB, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 201);
IPAddress ipServer(192, 168, 1, 200);
unsigned int localPort = 5005;
unsigned int appPortM = 8080;

String paquete = "";

// Buffer de envío y recepción
char packetBuffer[UDP_TX_PACKET_MAX_SIZE] = "";
char ReplyBuffer[UDP_TX_PACKET_MAX_SIZE] = "";

// Ethernet Udp Packet
EthernetUDP Udp;
#include <SD.h> // PARA LECTURA/ESCRITURA SD
File myFile;

const int pinRelay = 2;   // El pin del relé
const int pinMicrofono = 5; // El pin del módulo de micrófono

void setup() {
  Serial.begin(9600);
  pinMode(pinRelay, OUTPUT); // Configura el pin del relé como salida
  pinMode(pinMicrofono, INPUT); // Configura el pin del micrófono como entrada
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.print("SD");
  if (!SD.begin(4)) {
    return;
  }
  Serial.println("OK");
  SPI.begin();
}

void loop() {
  recibirUDP();
  // Lee el valor del micrófono
  int estadoMicrofono = digitalRead(pinMicrofono);

  Serial.print("Valor del micrófono: ");
  Serial.println(estadoMicrofono);
  
  // Si la señal de salida digital del módulo del micrófono es 1 (escucho un ruido significativo), enciende el relé
  if (estadoMicrofono == 1) {
    String mesage = "RUIDO;LED ON"; //mensaje a enviar por ethernet
    const char* charArray = mesage.c_str(); //lo convertimos a char
    enviarUDP(charArray,ipServer,appPortM); //lo enviamos
    digitalWrite(pinRelay, HIGH); // Enciende el relé
  } else {
    digitalWrite(pinRelay, LOW);  // Apaga el relé
  }
  delay(1000);
  Serial.println("Waiting");
}

void enviarUDP(char paquete[], IPAddress ip, unsigned int port) {
  Udp.beginPacket(ip, port); //Start building up a packet to send to the remote host specific in host and port
  Udp.write(paquete);
  int resultado = Udp.endPacket(); //returns 1 if successful, 0 if there was a problem resolving the hostname or port
  Serial.print("Resultado del envío: "); 
  if (resultado == 1){
    Serial.println("enviado con éxito");
  } else { 
    Serial.println("error");
  }
  limpiarBuffer();
}

void recibirUDP() {
  byte packetSize = Udp.parsePacket();
  String temp;
  if (packetSize) {
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    for (byte i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++) {
      temp = temp + String(packetBuffer[i]);
      if (temp == "RST") {
        asm volatile("jmp 0");
      }
    }
  }
}

void limpiarBuffer() {
  for (byte i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++) {
    packetBuffer[i] = 0;
    ReplyBuffer[i] = 0;
  }
}
