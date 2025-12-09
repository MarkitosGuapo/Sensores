#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <Servo.h>

// --- CONFIGURACIÓN RFID RC522 ---
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// --- CONFIGURACIÓN SERVO ---
Servo miServo;
#define SERVO_PIN A0

// --- CONFIGURACIÓN TECLADO 4x4 (S1..S16) ---
// Mapeo lógico (fila 1 = S1,S2,S3,S4; fila2 = S5..S8; etc.)
const byte FILAS = 4;
const byte COLUMNAS = 4;

char teclas[FILAS][COLUMNAS] = {
  {'1', '2', '3', 'A'},   // S1 S2 S3 S4
  {'4', '5', '6', 'B'},   // S5 S6 S7 S8
  {'7', '8', '9', 'C'},   // S9 S10 S11 S12
  {'*', '0', '#', 'D'}    // S13 S14 S15 S16
};

// fijamos pines que no interfieran con Serial ni RC522
byte pinesFilas[FILAS] = {2, 3, 4, 5};      // R1-R4  (S1..S4 están en R1 -> D2)
byte pinesColumnas[COLUMNAS] = {6, 7, 8, A1}; // C1-C4

Keypad teclado = Keypad(makeKeymap(teclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

// --- TARJETAS AUTORIZADAS ---
String tarjetasAutorizadas[] = {
  "C9 A2 65 A2",  // Tarjeta 1
  "B3 7A 5C 12"   // Tarjeta 2 (opcional)
};
const int numTarjetas = sizeof(tarjetasAutorizadas) / sizeof(tarjetasAutorizadas[0]);

// --- VARIABLES ---
String claveCorrecta = "ABCD";   // la contraseña que quieres (4 dígitos)
String claveIngresada = "";
bool tarjetaValida = false;
unsigned long tiempoLecturaPIN = 0;
const unsigned long tiempoMaxPIN = 20000; // 20s para ingresar PIN

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  miServo.attach(SERVO_PIN);
  miServo.write(0); // posición cerrada

  Serial.println("🔐 Sistema RFID + Teclado 4x4 + Servo (auto-open al completar PIN)");
  Serial.println("Pase una tarjeta para comenzar...");
}

void loop() {
  // --- DETECTAR TARJETA RFID ---
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String contenido = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      contenido += String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      contenido += String(rfid.uid.uidByte[i], HEX);
    }
    contenido.toUpperCase();
    Serial.print("UID detectado:");
    Serial.println(contenido);

    // Comprobar si la tarjeta está autorizada
    bool encontrada = false;
    for (int i = 0; i < numTarjetas; i++) {
      if (contenido.substring(1) == tarjetasAutorizadas[i]) {
        encontrada = true;
        break;
      }
    }

    if (encontrada) {
      Serial.println("✅ Tarjeta válida. Ahora ingrese la clave (usa S1..S4 para la fila 1).");
      tarjetaValida = true;
      claveIngresada = "";
      tiempoLecturaPIN = millis();
    } else {
      Serial.println("❌ Tarjeta NO autorizada.");
      tarjetaValida = false;
    }

    rfid.PICC_HaltA();
  }

  // --- SI ESTAMOS EN MODO PIN Y SE EXPIRO EL TIEMPO ---
  if (tarjetaValida && (millis() - tiempoLecturaPIN > tiempoMaxPIN)) {
    Serial.println("\n⏱ Tiempo para introducir PIN agotado. Pase tarjeta nuevamente.");
    tarjetaValida = false;
    claveIngresada = "";
  }

  // --- LECTURA DE TECLADO ---
  char k = teclado.getKey();
  if (k && tarjetaValida) {
    // borrar
    if (k == '*') {
      claveIngresada = "";
      Serial.println("\n🔁 Clave borrada.");
      return;
    }

    // ignoramos '#' porque quieres auto-open al completar longitud
    if (k == '#') {
      Serial.println("\n(Has presionado # — pero el sistema abre automáticamente al completar la longitud del PIN.)");
      return;
    }

    // añadir caracter (incluye A,B,C,D si se usan)
    claveIngresada += k;
    Serial.print("*"); // no mostrar la clave real

    // si la longitud alcanzó la de la clave correcta, verificar inmediatamente
    if (claveIngresada.length() >= claveCorrecta.length()) {
      Serial.println();
      if (claveIngresada.substring(0, claveCorrecta.length()) == claveCorrecta) {
        Serial.println("🔓 Clave correcta. Abriendo servo...");
        abrirServo();
      } else {
        Serial.println("❌ Clave incorrecta. Acceso denegado.");
      }
      // reset estado
      claveIngresada = "";
      tarjetaValida = false;
      Serial.println("Pase una tarjeta nuevamente...");
    }
  }
}

// --- FUNCIÓN PARA ABRIR PUERTA ---
void abrirServo() {
  miServo.write(90); // Abre
  delay(3000);       // Mantiene abierto (ajusta si quieres más/menos)
  miServo.write(0);  // Cierra
  Serial.println("🔒 Puerta cerrada.");
}
