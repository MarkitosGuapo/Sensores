#include <Servo.h>

Servo servoMotor;

#define TRIG 9
#define ECHO 8
#define SERVO_PIN 6

void setup() {
  Serial.begin(9600);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  servoMotor.attach(SERVO_PIN);
}

long leerUltrasonico() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracion = pulseIn(ECHO, HIGH);
  long distancia = duracion * 0.034 / 2;

  return distancia;
}

void loop() {
  int joystickX = analogRead(A0);
  long distancia = leerUltrasonico();

  int angulo = map(joystickX, 0, 1023, 0, 180);
  servoMotor.write(angulo);

  Serial.print(joystickX);
  Serial.print(",");
  Serial.println(distancia);

  delay(150);
}
