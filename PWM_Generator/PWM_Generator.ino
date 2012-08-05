#include <Servo.h>

Servo myservo;

const int potPin = 0;

int potInput = 0;
float pwmValue = 0;

unsigned long updateLast = 0;
unsigned long updateCurrent = 0;

void setup() {
  myservo.attach(3, 678, 2310);
  Serial.begin(9600);
}

void loop() {
//  if (Serial.available() > 0) {
//    Delay = 0;
//    while (Serial.available() > 0) {
//      Delay = Delay*10+Serial.read()-48;
//      delay(2);
//    }
//    highTime = map(Delay, 0, 1000, 600, 2400);
//    Serial.print("Input = ");
//    Serial.println(highTime);
//    Serial.print("High Time in Microseconds = ");
//    Serial.println(highTime);
//  }
  potInput = analogRead(potPin);
  pwmValue= map(potInput, 0, 1023, 0, 179); // For servo library
  myservo.write(pwmValue);
  updateCurrent = millis();
  if (updateCurrent - updateLast > 500) {
    updateLast = updateCurrent;
    pwmValue = map(pwmValue, 0, 179, -1, 1);
    printAnalog(potInput, pwmValue, updateCurrent, updateLast);
  }
}

void printAnalog(int input, float pwmValue, unsigned long updateCurrent, unsigned long updateLast) {
  Serial.print("Analog input = ");
  Serial.println(input);
  Serial.print("PWM Value = ");
  Serial.println(pwmValue);
}

