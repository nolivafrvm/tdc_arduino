/*
  UTN - Universidad 
*/


#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

byte mac[] = {0xDA, 0xAB, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 201);
IPAddress ipServerApp(192, 168, 1, 100);
unsigned int localPort = 5005; 
unsigned int appPortM = 8080;      

String paquete = "";

// Buffer de envio y recepción
char packetBuffer[UDP_TX_PACKET_MAX_SIZE] = "";
char ReplyBuffer[UDP_TX_PACKET_MAX_SIZE] = "";

// Ethernet Udp Packet
EthernetUDP Udp;
#include <SD.h> // PARA LECTURA/ESCRITURA SD
File myFile;

void setup() {
  Serial.begin(9600);
  pinMode(4, OUTPUT); // PARA TARJETA SD
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);  
  Serial.print("SD");
  if (!SD.begin(4))
  {
    return;
  }
  Serial.println("OK");
  SPI.begin();     
}

void loop()
{
  recibirUDP();
  delay(500);
  Serial.println("Waiting");

  // Ejemplo envio de paquete
  paquete = "accion;detalle";
  paquete.toCharArray(ReplyBuffer, 24);
  enviarUDP(ReplyBuffer, ipServerApp, appPortM);
}


void enviarUDP(char paquete[], IPAddress ip, unsigned int port)
{
  Udp.beginPacket(ip, port);
  Udp.write(paquete);
  Udp.endPacket();
 // for (int i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++) paquete[i] = 0; // FLUSHING UDP PACKET
  limpiarBuffer();
}


void recibirUDP()
{
  byte packetSize = Udp.parsePacket();
  String temp;
  if (packetSize)
  {
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    for (byte i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++)
    {
      temp = temp + String(packetBuffer[i]);      
      if (temp == "SL")
      {                
        limpiarBuffer();    
        i = UDP_TX_PACKET_MAX_SIZE;
      }

      if (temp == "RST") {
        asm volatile ("jmp 0");
      }
    }
  }
}
void limpiarBuffer()
{
  for (byte i = 0; i < UDP_TX_PACKET_MAX_SIZE; i++)
  {
    packetBuffer[i] = 0;
    ReplyBuffer[i]= 0;
  }
}



