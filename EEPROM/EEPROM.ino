#include <EEPROM.h>

String myString = "192.168.1.13"; // El String que deseas guardar
int EEPROMAddress = 0; // Dirección de inicio en la EEPROM donde se almacenará el String


void writeStringToEEPROM(int addr, String data) {
  int dataSize = data.length() + 1; // Tamaño de la cadena más el carácter nulo
  char charBuf[dataSize]; // Crea un array de caracteres para almacenar la cadena
  data.toCharArray(charBuf, dataSize); // Convierte el String en un array de caracteres
  for (int i = 0; i < dataSize; i++) {
    EEPROM.write(addr + i, charBuf[i]); // Escribe cada carácter en la EEPROM
  }
  EEPROM.commit(); // Guarda los datos en la EEPROM
}

void readStringFromEEPROM(int addr, char* buffer, int bufferSize) {
  for (int i = 0; i < bufferSize; i++) {
    buffer[i] = EEPROM.read(addr + i); // Lee cada carácter de la EEPROM
  }
}

void setup() {
  // Inicializa la comunicación con la EEPROM
  EEPROM.begin(512); // Puedes ajustar el tamaño de acuerdo a tus necesidades

  // Guarda el String en la EEPROM
  writeStringToEEPROM(EEPROMAddress, myString);

  // Lee el String de la EEPROM y lo imprime
  char readBuffer[myString.length() + 1];
  readStringFromEEPROM(EEPROMAddress, readBuffer, sizeof(readBuffer));
  Serial.begin(9600);
  Serial.println("String recuperado de la EEPROM:");
  Serial.println(readBuffer);
}

void loop() {
  // Tu código loop aquí
}
