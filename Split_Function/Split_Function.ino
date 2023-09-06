#include <Arduino.h>

void splitString(const String &input, char delimiter, String tokens[], int maxTokens) {
  int tokenIndex = 0; // Índice actual de tokens
  int startIndex = 0; // Índice de inicio de cada token

  // Recorre la cadena de entrada
  for (int i = 0; i < input.length(); i++) {
    char currentChar = input.charAt(i);

    // Si encontramos el delimitador o llegamos al final de la cadena
    if (currentChar == delimiter || i == input.length() - 1) {
      // Calcula la longitud del token
      int tokenLength = i - startIndex + 1;

      // Verifica si hemos alcanzado el límite máximo de tokens
      if (tokenIndex < maxTokens) {
        // Extrae el token de la cadena de entrada
        String token = input.substring(startIndex, startIndex + tokenLength);

        // Almacena el token en el arreglo de tokens
        tokens[tokenIndex] = token;

        // Incrementa el índice de tokens
        tokenIndex++;
      }

      // Actualiza el índice de inicio para el próximo token
      startIndex = i + 1;
    }
  }
}

void setup() {
  Serial.begin(9600);
  String inputString = "1;2.0;1.0;3.0;16.0;192.168.1.107;8080;127.0.0.1;8081";
  const int maxTokens = 10; // Máximo número de tokens esperados
  String tokens[maxTokens]; // Arreglo para almacenar los tokens

  // Llama a la función para separar la cadena de texto
  splitString(inputString, ';', tokens, maxTokens);

  // Imprime los tokens separados
  for (int i = 0; i < maxTokens; i++) {
    if (tokens[i].length() > 0) {
      Serial.print("Token ");
      Serial.print(i);
      Serial.print(": ");
      Serial.println(tokens[i]);
    }
  }
}

void loop() {
  // Tu código loop aquí
}
