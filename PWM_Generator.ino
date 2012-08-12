#include <Servo.h>
#include "ModePins.h"

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
int potVal1 = 0;
int potVal2 = 0;

// Stores output "angles" for the servo library, set to 90 because that's neutral
int outputVal1 = 90;
int outputVal2 = 90;

// Stores setting of mode selections switches
int syncSwitch = 0;
int reverseSwitch = 0;
int functionSwitch = 0;

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
  for (int iii=startPin; iii<=endPin; iii++) { // startPin and endPin are from ModePins.h
    pinMode(iii, INPUT);
  }
}

void loop() {
  // Gets potentiometer values
  potVal1 = analogRead(potPin1);
  potVal2 = analogRead(potPin2);
  
  // Maps potentiometer values to the servo angle from 0 to 180
  outputVal1 = map(potVal1, 0, 1023, 0, 180);
  outputVal2 = map(potVal2, 0, 1023, 0, 180);
 
  // Writes output values
  motor1.write(outputVal1);
  motor2.write(outputVal2);
  
  // Reads mode selector switch
  syncSwitch = modeSwitchRead(syncMode1, syncMode2);
  reverseSwitch = modeSwitchRead(reverseMode, reverseMode);
  functionSwitch = modeSwitchRead(normalMode, servoMode);
  
  // Sets function, like normal, sweep, servo
//  outputVal1 = setFunction(functionSwitch, outputVal1);
//  outputVal2 = setFunction(functionSwitch, outputVal2);
  
  // Changes output according to sync and reverse switches
  outputVal1 = setSync(syncSwitch, reverseSwitch, outputVal1, outputVal2);
  outputVal2 = setSync(syncSwitch, reverseSwitch, outputVal2, outputVal1);

  

  // Prints data for debugging
  updateLast = printAnalog(potVal1, outputVal1, updateCurrent, updateLast, serialDelay, 0, 1);
  updateLast = printAnalog(potVal2, outputVal2, updateCurrent, updateLast, serialDelay, 1, 2);  
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
    Serial.print("Output angle = ");
    Serial.println(servoAngle);
    Serial.println();
    if (resetDelayTimer == 1) {
      updateLast = updateCurrent;
    }
  }
  return updateLast;
}

int modeSwitchRead(int firstPin, int lastPin) { // Set first and last pins it reads from, iteratively reads from pins between them
  int modeSwitch = 0;
  for (int iii=firstPin; iii<=lastPin; iii++) {
    if (digitalRead(iii) == 1) {
      int modeSwitch = iii;
    }
  }
  return modeSwitch;
}

//int setFunction(int functionSwitch, outputVal

int setSync(int syncSwitch, int reverseSwitch, int primaryOutput, int secondaryOutput) {
  switch (syncSwitch) {
    case 0:
      return primaryOutput;
      break;
    case syncMode1:
      if (reverseSwitch == reverseMode)
        return reverseOutput(primaryOutput);
      else
        return primaryOutput;
      break;
    case syncMode2:
      if (reverseSwitch == reverseMode)
        return reverseOutput(secondaryOutput);
      else
        return secondaryOutput;
      break;
  }
}

int reverseOutput(int outputVal) {
  return (180-outputVal);
}

int sweepOutput(int outputVal) {
  return outputVal;
}
