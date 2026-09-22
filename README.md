[readme_md.md](https://github.com/user-attachments/files/32533976/readme_md.md)
# 💡 Control de Iluminación Vía Bluetooth con ESP32

[![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue?logo=espressif&logoColor=white)](https://www.espressif.com/)
[![Arduino IDE](https://img.shields.io/badge/IDE-Arduino%20IDE-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

Un proyecto IoT para el control remoto de cargas de alta potencia (bombillos, electrodomésticos) mediante conexión **Bluetooth Classic** usando un microcontrolador ESP32 y un teléfono móvil.

---

## 📌 Tabla de Contenidos

- [Características](#-características)
- [Diagrama de Trabajo y Transistor NPN](#-diagrama-de-trabajo-y-transistor-npn)
- [Materiales Requeridos](#-materiales-requeridos)
- [Conexiones Hardware](#-conexiones-hardware)
- [Explicación del Código](#-explicación-del-código)
- [Configuración de la App Móvil](#-configuración-de-la-app-móvil)
- [Instalación y Uso](#-instalación-y-uso)
- [Licencia](#-licencia)

---

## ✨ Características

- **Control Remoto:** Encendido y apagado inalámbrico mediante comandos `ON` y `OFF`.
- **Adaptación de Impedancia y Voltaje:** Etapa de acoplamiento de 3.3V (GPIO) a 5V (Relé) mediante transistor NPN 2N3904.
- **Doble Indicador:** Encendido simultáneo del relé y del LED en placa (`GPIO 2`) para monitoreo visual de estado.
- **Comunicación Bidireccional:** Monitoreo del estado a través de la terminal Serial a `115200` baudios.

---

## ⚡ Diagrama de Trabajo y Transistor NPN

El ESP32 opera con lógica de **3.3V** en sus pines GPIO y puede suministrar una corriente máxima recomendada de **12 mA** por pin. Por otro lado, la bobina del relé requiere **5V** y una corriente mayor para conmutar.

### ¿Por qué se utiliza el Transistor 2N3904 y las 2 Resistencias?

1. **Aislamiento y Amplificación de Corriente:** El transistor actúa como un conmutador electrónico (*switch*). El GPIO 26 solo suministra una pequeña corriente a la base del transistor, permitiendo que la corriente principal fluya desde el pin `VIN` (5V) hacia el relé.
2. **Acondicionamiento de Señal (Resistencias 1kΩ):**
   - **$R_1$ (Base):** Limita la corriente proveniente del GPIO 26 para proteger el pin del ESP32 y evitar saturar en exceso la base del transistor.
   - **$R_2$ (Pull-down):** Asegura que la base permanezca en nivel bajo (`LOW`) durante el arranque del ESP32, evitando que el relé se active de forma errática mientras el microcontrolador se inicializa.

---

## 🛠️ Materiales Requeridos

| Cantidad | Componente | Descripción |
| :---: | | |
| 1 | **ESP32** | Placa de desarrollo NodeMCU o equivalente |
| 1 | **Módulo Relé 5V** | Relevador electromecánico de 1 canal |
| 1 | **Transistor 2N3904** | Transistor NPN de propósito general |
| 2 | **Resistencias 1 kΩ** | 1/4W |
| 1 | **Bombillo y Plafón** | Carga de red de corriente alterna (110V/220V) |
| 1 | **Smartphone** | Con Bluetooth Classic |
| - | **Jumpers y Protoboard** | Para la interconexión de componentes |

---

## 🔌 Conexiones Hardware

### Circuito de Control (Bajo Voltaje)

| ESP32 Pin | Componente Destino | Función |
| :--- | :--- | :--- |
| **VIN** | VCC del Módulo Relé | Alimentación a 5V (tomados del puerto USB) |
| **GND** | GND del Relé y Emisor del 2N3904 | Tierra común (GND) |
| **GPIO 26** | Resistencia de 1kΩ a Base del 2N3904 | Señal de control digital |
| **Collector (2N3904)** | Entrada IN / Control del Relé | Activa la señal del relé al saturarse |

### Circuito de Carga (Alto Voltaje)

> ⚠️ **¡ADVERTENCIA DE SEGURIDAD!** 
> Trabajar con corriente alterna (110V/220V) puede ser peligroso. Asegúrate de que el circuito no esté conectado a la red eléctrica mientras realizas el cableado.

```
Fase (AC)  ────▶ [ COM ] (Relé)
                  [ NO  ] (Relé) ────▶ Plafón/Bombillo ────▶ Neutro (AC)
```

---

## 💻 Explicación del Código

El firmware está desarrollado en **C++** para el ecosistema Arduino. A continuación se desglosa el funcionamiento interno de cada sección:

### 1. Inclusión de Librerías y Definición de Variables

```cpp
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

const int LED = 2;     // LED azul incorporado en la placa ESP32
const int Relay = 26;  // Pin GPIO asignado al control del relé
```
- `#include <BluetoothSerial.h>`: Importa el stack de Bluetooth Classic del ESP32.
- `BluetoothSerial SerialBT`: Crea la instancia del objeto para administrar la comunicación serie por Bluetooth.
- Definimos variables `const int` para evitar números "mágicos" en el código y facilitar cambios de pines en el futuro.

### 2. Configuración Inicial (`setup`)

```cpp
void setup() {
  pinMode(LED, OUTPUT);
  pinMode(Relay, OUTPUT);

  // Asegura el estado apagado al iniciar
  digitalWrite(LED, LOW);
  digitalWrite(Relay, LOW);

  Serial.begin(115200);
  SerialBT.begin("ESP32_Luz"); // Nombre visible del Bluetooth
}
```
- `pinMode()`: Configura los pines del LED y del relé como salidas digitales.
- `digitalWrite(..., LOW)`: Garantiza que la lámpara permanezca apagada cuando el dispositivo se enciende o reinicia.
- `SerialBT.begin("ESP32_Luz")`: Inicializa la pila Bluetooth y asigna el nombre con el que aparecerá el dispositivo en la búsqueda móvil.

### 3. Bucle Principal (`loop`)

```cpp
void loop() {
  if (SerialBT.available()) {
    // Lee la cadena hasta encontrar un salto de línea
    String mensaje = SerialBT.readStringUntil('\n');
    mensaje.trim(); // Elimina caracteres '\r', espacios o saltos de línea sobrantes

    if (mensaje == "ON") {
      digitalWrite(LED, HIGH);
      digitalWrite(Relay, HIGH);
      Serial.println("Estado: ENCENDIDO");
    } 
    else if (mensaje == "OFF") {
      digitalWrite(LED, LOW);
      digitalWrite(Relay, LOW);
      Serial.println("Estado: APAGADO");
    }
  }

  // Reenvío de comandos del Monitor Serial al Bluetooth
  while (Serial.available()) {
    char c = Serial.read();
    SerialBT.write(c);
  }
}
```

#### ¿Cómo funciona el procesamiento de comandos?
1. **`SerialBT.available()`**: Verifica si han llegado bytes por Bluetooth.
2. **`readStringUntil('\n')`**: Acumula los caracteres recibidos hasta que detecta el carácter del salto de línea (`\n`), formando la cadena completa (ej. `"ON\n"`).
3. **`mensaje.trim()`**: Esencial para limpiar residuos invisibles como el retorno de carro (`\r`), dejando únicamente el texto puro `"ON"` o `"OFF"`.
4. **Comparación estricta (`==`)**: Si coincide con `"ON"`, se envía un nivel alto (`HIGH`) a los pines `2` y `26`. Si es `"OFF"`, se manda nivel bajo (`LOW`). Esto previene falsas activaciones ante cualquier otro texto aleatorio.

---

## 📲 Configuración de la App Móvil

Para controlar el sistema se recomienda usar **Serial Bluetooth Terminal** (disponible en Android):

1. Abre la app y accede a **Devices** para vincularte a **`ESP32_Luz`**.
2. Ve a la sección de **Settings > Macro** para configurar botones de acceso rápido (*shortcuts*):
   - **Macro 1:** Nombre: `ENCENDER` | Value: `ON` (Asegúrate de tener configurado `Newline` como `LF` o `CR+LF`).
   - **Macro 2:** Nombre: `APAGAR` | Value: `OFF`.

---

## 🚀 Instalación y Uso

1. Clona este repositorio:
   ```bash
   git clone https://github.com/tu-usuario/esp32-bluetooth-relay.git
   ```
2. Abre el archivo `.ino` dentro de **Arduino IDE**.
3. Selecciona la tarjeta **ESP32 Dev Module** y el puerto COM correspondiente.
4. Carga el código a la placa.
5. Abre el Monitor Serial a `115200 baudios` para verificar el estado de conexión.

---

## 📄 Licencia

Este proyecto está bajo la Licencia MIT. Consulta el archivo `LICENSE` para más detalles.
