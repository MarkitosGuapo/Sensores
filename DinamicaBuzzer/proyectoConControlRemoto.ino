#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

// --- LCD I2C conectado a A4 (SDA) y A5 (SCL) ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Sensor IR en pin 2 ---
#define IR_RECEIVE_PIN 2

// --- Buzzer en pin 6 ---
#define BUZZER_PIN 6

// --- Códigos IR capturados de tu control ---
#define CODIGO_1 0xEE11FB04 // Botón 1
#define CODIGO_2 0xED12FB04  // Botón 2
#define CODIGO_3 0xEC13FB04  // Botón 3

// --- Variables para secuencia 1-2-3 ---
int secuencia[3] = {0, 0, 0};
int posicion = 0;

// --- Estado de la alarma ---
bool alarmaActiva = false;
unsigned long tiempoUltimaSenal = 0;

void setup() {
  // Serial Monitor
  Serial.begin(9600);
  Serial.println("=== SISTEMA DE ALARMA CON SECUENCIA 1-2-3 ===");
  
  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("Buzzer configurado");
  
  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ingrese codigo");
  lcd.setCursor(0, 1);
  lcd.print("Secuencia: 1-2-3");
  Serial.println("LCD configurado");

  // IR
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.println("IR configurado");
  
  Serial.println("=== SISTEMA LISTO ===");
  Serial.println("Ingrese secuencia 1-2-3 para acceso");
  Serial.println();
}

void loop() {
  if (IrReceiver.decode()) {

    // Anti-rebote
    if (millis() - tiempoUltimaSenal < 300) {
      IrReceiver.resume();
      return;
    }
    tiempoUltimaSenal = millis();

    uint32_t codigo = IrReceiver.decodedIRData.decodedRawData;
    
    Serial.print(">>> Código recibido: 0x");
    Serial.println(codigo, HEX);

    // Identificar qué botón se presionó
    int botonPresionado = 0;
    
    if (codigo == CODIGO_1) {
      botonPresionado = 1;
      Serial.println("-> Botón 1 detectado");
    }
    else if (codigo == CODIGO_2) {
      botonPresionado = 2;
      Serial.println("-> Botón 2 detectado");
    }
    else if (codigo == CODIGO_3) {
      botonPresionado = 3;
      Serial.println("-> Botón 3 detectado");
    }

    // Si se presionó 1, 2 o 3, agregar a la secuencia
    if (botonPresionado >= 1 && botonPresionado <= 3) {
      agregarASecuencia(botonPresionado);
      
      // Mostrar progreso en LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Codigo: ");
      for (int i = 0; i < posicion; i++) {
        lcd.print(secuencia[i]);
      }
      
      // Verificar si la secuencia es 1-2-3
      if (verificarSecuencia()) {
        // ACCESO EXITOSO
        Serial.println("************");
        Serial.println("* ACCESO EXITOSO - CODIGO CORRECTO *");
        Serial.println("************");
        
        alarmaActiva = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACCESO EXITOSO");
        lcd.setCursor(0, 1);
        lcd.print("Bienvenido!");
        
        // Beep de éxito (tono corto y agudo)
        for (int i = 0; i < 2; i++) {
          digitalWrite(BUZZER_PIN, HIGH);
          delay(100);
          digitalWrite(BUZZER_PIN, LOW);
          delay(100);
        }
        
        delay(3000);
        resetearSecuencia();
        
        lcd.clear();
        lcd.print("Sistema activo");
      }
      else if (posicion >= 3) {
        // SECUENCIA INCORRECTA - ACCESO DENEGADO
        Serial.println("************");
        Serial.println("* ACCESO DENEGADO - CODIGO INCORRECTO *");
        Serial.println("************");
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACCESO DENEGADO");
        lcd.setCursor(0, 1);
        lcd.print("Codigo invalido!");
        
        // Buzzer de error (sonido largo y grave)
        IrReceiver.resume();
        
        for (int i = 0; i < 3; i++) {
          digitalWrite(BUZZER_PIN, HIGH);
          delay(500);  // Sonido más largo
          digitalWrite(BUZZER_PIN, LOW);
          delay(200);
        }
        
        delay(2000);
        resetearSecuencia();
        
        lcd.clear();
        lcd.print("Ingrese codigo");
        lcd.setCursor(0, 1);
        lcd.print("Secuencia: 1-2-3");
      }
    }

    IrReceiver.resume();
    delay(100);
  }
}

void agregarASecuencia(int boton) {
  if (posicion < 3) {
    secuencia[posicion] = boton;
    posicion++;
    Serial.print("Secuencia actual: ");
    for (int i = 0; i < posicion; i++) {
      Serial.print(secuencia[i]);
      Serial.print(" ");
    }
    Serial.println();
  }
}

bool verificarSecuencia() {
  // Verificar si la secuencia es exactamente 1-2-3
  if (posicion == 3) {
    if (secuencia[0] == 1 && secuencia[1] == 2 && secuencia[2] == 3) {
      return true;
    }
  }
  return false;
}

void resetearSecuencia() {
  posicion = 0;
  secuencia[0] = 0;
  secuencia[1] = 0;
  secuencia[2] = 0;
  Serial.println("Secuencia reseteada");
}