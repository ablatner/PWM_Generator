#include <Servo.h>
#include "ModePins.h"
#include "Signum.h"

// Sets cycle time
const double maxChangePerSecond = 180; // max motor speed change per second on the 0-180 scale
const double maxChangePerCycle = 1; // probably should just leave at 1, for max resolution with respect to time
const int timePerCycle = ceil(maxChangePerCycle*1000.0/maxChangePerSecond); // take ceiling of result for safety, slower changes; (change/cycle)*(1000ms/second)/(change/second) = ms/cycle

// Sweep parameters
const unsigned long minSweepTime =5000; // the minimum time for 1 period of sweep
const unsigned long maxSweepTime = 60000; // the maximum time for 1 period of sweep
// constant for use in sweep equation that adjusts the input to give values between min and max, equivalent to the change in sweep time per degree of pot-turn
const float sweepInputMultiplier = (float)(maxSweepTime-minSweepTime)/180.0;

const int neutralRange = 10; // Range above or below the pot input that corresponds to neutral, means that 512+/- neutral range would still be neutral

// Initialize PWM signals
Servo motor1;
Servo motor2;

// Set analog input pins for potentiometers
const int potPin1 = 0;
const int potPin2 = 1;

// Set pin for talon selection
const int talonPin = 7;
boolean lastTalonSwitch = false;
boolean currentTalonSwitch = false;

// Stores raw potentiometer readings, set to 512 because that's neutral
int potVal1 = 512;
int potVal2 = 512;

// Set PWM output pins
const int outputPin1 = 5;
const int outputPin2 = 6;

// Stores value after mapping, set to 90 because that's neutral
int mapVal1 = 90;
int mapVal2 = 90;

// Stores output "angles", set to 90 because that's neutral
int outputVal1 = 90;
int outputVal2 = 90;

// Stores setting of mode selections switches
int modeSwitch = 0;

// For smoothing, set to 90 because that's neutral
int lastOutput1 = 90;
int lastOutput2 = 90;

//----------- DEBUGGING STUFF ----------- when in use there can be slight pauses when it outputs, disable for real use
//// For limiting serial output to a low frequency
//unsigned long updateLast = 0;
//unsigned long updateCurrent = 0;
//int serialDelay = 400; // Waits this many milliseconds between data outputs
//
//// Prints both raw analog values and actual output values over serial, for debugging
//// Updates if current time minus last time is greater than the delay time
//// Only resets delay timer when "resetDelayTimer" is true
//// Set which motor, input, and output number is displayed with "textNumber"
//unsigned long printAnalog(int analogInput, int servoAngle, int modeSwitch, unsigned long updateCurrent, unsigned long updateLast, int Delay, boolean resetDelayTimer, int textNumber) {
//  updateCurrent = millis();
//  if (updateCurrent - updateLast > Delay) {
////    Serial.print("-");Serial.println(textNumber);
////    Serial.print("M ");Serial.println(modeSwitch);
//    Serial.print("I ");Serial.println(analogInput);
////    Serial.print("O ");Serial.println(servoAngle);
//    Serial.println();
//    if (resetDelayTimer == 1)
//      updateLast = updateCurrent;
//  }
//  return updateLast;
//}

void setup() {
  Serial.begin(9600);
  
  // Initialize mode switch input pins, "for loop" uses less memory
  for (int iii=startPin; iii<=endPin; iii++) { // startPin and endPin are from ModePins.h
    pinMode(iii, INPUT_PULLUP);
  }
  
  // Initialize talon selection switch
  pinMode(talonPin, INPUT_PULLUP);
  lastTalonSwitch = digitalRead(talonPin);
  currentTalonSwitch = digitalRead(talonPin);
}

void loop() {
  // Gets potentiometer values
  potVal1 = analogRead(potPin1);
  if ((512-neutralRange < potVal1) && (potVal1 < neutralRange+512)) // Easily set to neutral
    potVal1 = 512;
  potVal2 = analogRead(potPin2);
  if ((512-neutralRange < potVal2) && (potVal2 < neutralRange+512))
    potVal2 = 512;
 
  // Reads mode switch
  modeSwitch = modeSwitchRead(startPin, endPin);
 
  // Sets talon mode and pwm ranges
  currentTalonSwitch = digitalRead(talonPin);
  if (lastTalonSwitch != currentTalonSwitch) { // checks if talon mode has changed
    if (currentTalonSwitch) { // talon mode on
      motor1.attach(outputPin1, 900, 2000);
      motor1.attach(outputPin2, 900, 2000);
    }
    else { // talon mode off, for victors, jaguars, Hitec HS-322HD servos
      motor1.attach(outputPin1, 678, 2310);
      motor2.attach(outputPin2, 678, 2310);
    }
  }
 
  // Maps potentiometer values to the servo angle from 0 to 180
  mapVal1 = map(potVal1, 0, 1023, 0, 180);
  mapVal2 = map(potVal2, 0, 1023, 0, 180);

  // Sets mode
  outputVal1 = setMode(modeSwitch, mapVal1, mapVal2, 1);
  outputVal2 = setMode(modeSwitch, mapVal1, mapVal2, 2);

  // Output smoothing
  outputVal1 = smooth(outputVal1, lastOutput1, maxChangePerCycle);
  outputVal2 = smooth(outputVal2, lastOutput2, maxChangePerCycle);

  // Prints data for debugging
//  updateLast = printAnalog(potVal1, outputVal1, modeSwitch, updateCurrent, updateLast, serialDelay, 1, 1);
//  updateLast = printAnalog(potVal2, outputVal2, modeSwitch, updateCurrent, updateLast, serialDelay, 1, 2);

  // Writes output values
  motor1.write(outputVal1);
  motor2.write(outputVal2);

  // Sets last output
  lastOutput1 = outputVal1;
  lastOutput2 = outputVal2;

  delay(timePerCycle);
}

int modeSwitchRead(int startPin, int endPin) { // Set first and last pins it reads from, iteratively reads from pins between them
  int modeSwitch = -1; // If returns -1, read error
  for (int iii=endPin; iii<=startPin; iii--) { // start with servo mode, end at independent mode so independent overrides all
    if (digitalRead(iii) == 0) // switch connects to gnd and there are internal pullups, so when it's switched, it goes low
      modeSwitch = iii;
  }
  return modeSwitch;
}

int setMode(int modeSwitch, int outputVal1, int outputVal2, int outputSelect) {  
  switch (outputSelect) {
    case 1:
      switch (modeSwitch) {
        case independentMode:
          return outputVal1;
          break;
        case syncMode:
          return outputVal1;
          break;
        case syncReverseMode:
          return outputVal1;
          break;
        case sweepMode:
          return sweepOutput(outputVal1);
          break;
        case sweepReverseMode:
          return sweepOutput(outputVal1);
          break;
        case servoMode:
          return servoOutput(outputVal1);
          break;
        default:
          return outputVal1;
      }
    case 2:
      switch (modeSwitch) {
        case independentMode:
          return outputVal2;
          break;
        case syncMode:
          return outputVal1;
          break;
        case syncReverseMode:
          return reverseOutput(outputVal1);
          break;
        case sweepMode:
          return sweepOutput(outputVal1);
          break;
        case sweepReverseMode:
          return reverseOutput(sweepOutput(outputVal1));
          break;
        case servoMode:
          return servoOutput(outputVal2);
          break;
        default:
          return outputVal2;
      }
     default:
       return 90; // returns neutral if improper output selected
  }
}

int reverseOutput(int outputVal) {
  return (180-outputVal);
}
  
int sweepOutput(int outputVal) {
  float time = millis();
  // multiply by 1000 before setting to int value because it would be -1, 0, or 1 otherwise and lose value
  // factor 2pi shrinks period from 2pi to 1
  // factor 1/(...) extends period 1 to the (minimum time)+(pot-turn)*(change in sweep time per degree of pot-turn) = (time) + (angle)*(time/angle) = time
  // setting outputVal to 0 gives factor 1/(minSweepTime)
  // setting outputVal to 180 gives factor 1/(minSweepTime + 180*(maxSweepTime-minSweepTime)/180.0) = 1/(min + max - min) = 1/max
  outputVal = 1000*sin(time*6.2832/(minSweepTime+(float)outputVal*sweepInputMultiplier));
  outputVal = map(outputVal, -1000, 1000, 0, 180); // from +- 1000 because it can only take ints, and sin() makes -1 to 1
  return outputVal;
}

int servoOutput(int outputVal) {
  return outputVal;
}

int smooth(int outputVal, int lastOutput, int maxChangePerCycle) {
  if (fabs(outputVal-lastOutput) < maxChangePerCycle)
    return outputVal;
  else
    return lastOutput+sign(outputVal-lastOutput)*maxChangePerCycle;
}
