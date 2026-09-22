#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

const int LED = 2;
const int Relay = 26;

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(Relay, OUTPUT);

  // Garantizar estado inicial apagado
  digitalWrite(LED, LOW);
  digitalWrite(Relay, LOW);

  Serial.begin(115200);
  SerialBT.begin("ESP32_EBSAM");
}

void loop() {
  // Procesar datos provenientes del Bluetooth
  if (SerialBT.available()) {
    // Leer hasta encontrar un salto de línea o retorno de carro
    String mensaje = SerialBT.readStringUntil('\n');
    
    // Limpiar espacios en blanco, '\r' o saltos de línea
    mensaje.trim();

    if (mensaje == "ON") {
      digitalWrite(LED, HIGH);
      digitalWrite(Relay, HIGH);
      Serial.println("Estado: ENCENDIDO (ON)");
    } 
    else if (mensaje == "OFF") {
      digitalWrite(LED, LOW);
      digitalWrite(Relay, LOW);
      Serial.println("Estado: APAGADO (OFF)");
    }
  }

  // Procesar datos provenientes del Monitor Serial hacia el Bluetooth
  while (Serial.available()) {
    char c = Serial.read();
    SerialBT.write(c);
  }
}