#include <Servo.h>

// Initialize PWM signals
Servo motor1;
Servo motor2;

// Set analog input pins for potentiometers
const int potPin1 = 0;
const int potPin2 = 1;

// Set PWM output pins
const int outputPin1 = 5;
const int outputPin2 = 6;

// Stores raw potentiometer readings
int potInput1 = 0;
int potInput2 = 0;

// Stores output "angles" for the servo library, set to 90 because that's neutral
int outputVal1 = 90;
int outputVal2 = 90;

// Stores mode selection switch value
int modeSwitch = 0;

// Names mode switch input pins
const int independent = 8;
const int sync = 9;
const int syncReverse = 10;
const int sweep = 11;
const int sweepReverse = 12;
const int servo = 13;
struct modePins{
  int independent;
  int sync;
  int syncReverse;
  int sweep;
  int sweepReverse;
  int servo;
};
const modePins modePins = {independent, sync, syncReverse, sweep, sweepReverse, servo};

// For limiting serial output to a low frequency
unsigned long updateLast = 0;
unsigned long updateCurrent = 0;
int serialDelay = 2000; // Waits this many milliseconds between data outputs

void setup() {
  Serial.begin(9600);
  
  // Initialize PWM outputs
  motor1.attach(outputPin1, 678, 2310);
  motor2.attach(outputPin2, 678, 2310);

  // Initialize mode switch input pins, "for loop" uses less memory
  for (int iii=8; iii<14; iii++) {
    pinMode(iii, INPUT);
  }
}

void loop() {
  // Gets potentiometer values
  potInput1 = analogRead(potPin1);
  potInput2 = analogRead(potPin2);
  
  // Maps potentiometer values to the servo angle from 0 to 180
  outputVal1 = map(potInput1, 0, 1023, 0, 180);
  outputVal2 = map(potInput2, 0, 1023, 0, 180);
 
  // Writes output values
  motor1.write(outputVal1);
  motor2.write(outputVal2);
  
  // Reads mode selector switch
  modeSwitch = modeSwitchRead();
  
  // Prints data for debugging
  updateLast = printAnalog(potInput1, outputVal1, updateCurrent, updateLast, serialDelay, 0, 1);
  updateLast = printAnalog(potInput2, outputVal2, updateCurrent, updateLast, serialDelay, 1, 2);  
}

// Prints both raw analog values and actual output values over serial, for debugging
// Updates if current time minus last time is greater than the delay time
// Only resets delay timer when "resetDelayTimer" is true
// Set which motor, input, and output number is displayed with "textNumber"
unsigned long printAnalog(int analogInput, int servoAngle, unsigned long updateCurrent, unsigned long updateLast, int Delay, boolean resetDelayTimer, int textNumber) {
  updateCurrent = millis();
  if (updateCurrent - updateLast > Delay) {
    Serial.print("---- Data for output ");
    Serial.print(textNumber);
    Serial.println(" ----");    
    Serial.print("Analog input = ");
    Serial.println(analogInput);
    Serial.print("Servo angle = ");
    Serial.println(servoAngle);
    Serial.println();
    if (resetDelayTimer == 1) {
      updateLast = updateCurrent;
    }
  }
  return updateLast;
}

int modeSwitchRead() {
  for (int iii=8; iii<14; iii++) {
    if (digitalRead(iii) == 1) {
      int modeSwitch = iii;
    }
  }
  return modeSwitch;
}

int setMode(int modeSwitch, struct modePins) {
  
}
