/**
 * CAN MREX main (Template) file 
 *
 * File:            main.ino
 * Organisation:    MREX
 * Author:          Chiara Gillam
 * Date Created:    5/08/2025
 * Last Modified:   9/09/2025
 * Version:         1.1.3
 *
 */

#include "driver/twai.h"
#include "CM_Handler.h"
#include "CM_Transmit.h"
#include "CM_ObjectDictionary.h"
#include "CM_PDO.h"
#include "CM_Config.h"


// User code begin: ------------------------------------------------------

uint8_t nodeID = 1;  // Change this to set your device's node ID 

// --- Pin Definitions ---
// Can Bus
#define TX_GPIO_NUM GPIO_NUM_5 // Set GPIO pin for CAN Transmit
#define RX_GPIO_NUM GPIO_NUM_4 // Set GPIO pins for CAN Receive

//Motor control
const byte encoderPin = 2;            // Encoder signal pin
const byte motorPWMPin = 9;           // Motor PWM output pin
const unsigned int pulsesPerRev = 40; // Encoder pulses per revolution

// --- OD definitions ---
uint32_t desiredSpeed;
uint32_t trueSpeed;
uint8_t operatingMode;

// ======== Config ========


// ======== PID Settings ========
float Kp = 2.0;
float Ki = 0.5;
float Kd = 0.1;

// ======== Encoder Variables ========
volatile unsigned long pulseCount = 0;
volatile unsigned long lastPulseTime = 0;
volatile unsigned long minGap = 500; // microseconds (debounce)

// ======== PID Variables ========
float integral = 0; // We'll likely have these in the object dictionary as that will allow us to change it on the go with the can logger
float lastError = 0;
unsigned long lastPIDTime = 0;

// ======== Interrupt Service Routine ========
void onPulse() {
  unsigned long now = micros();
  unsigned long gap = now - lastPulseTime;

  if (gap >= minGap) {
    pulseCount++;
    lastPulseTime = now;

    // Adapt debounce dynamically
    //This is to account for rotary encoder pulsing extra times.
    minGap = (gap * 70UL) / 100UL;
    if (minGap < 50) minGap = 50;
  }
}



//OPTIONAL: timing for a non blocking function occuring every two seconds
// unsigned long previousMillis = 0;
// const long interval = 2000; // 2 seconds

// User code end ---------------------------------------------------------



void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("CAN MREX intialising over (TWAI)");

  //Initialize CANMREX protocol
  initCANMREX(TX_GPIO_NUM, RX_GPIO_NUM);

  //Initializes all TPDOs and RPDOs as disabled and clears runtime state
  initDefaultPDOs(nodeID);

  //Setup OD with default entries
  initDefaultOD();

  // User code Setup Begin: -------------------------------------------------
  // --- Register OD entries ---
  registerODEntry(0x60FF, 0x01, 2, sizeof(desiredSpeed), &desiredSpeed);
  registerODEntry(0x606C, 0x01, 2, sizeof(trueSpeed), &trueSpeed);
  registerODEntry(0x6060, 0x00, 2, sizeof(operatingMode), &operatingMode);

  pinMode(encoderPin, INPUT_PULLUP);
  pinMode(motorPWMPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(encoderPin), onPulse, RISING);
  lastPIDTime = millis();

  // User code Setup end ------------------------------------------------------


}

void loop() {
  handleCAN(nodeID); // Handles all incoming can messages
  //User Code begin loop() ----------------------------------------------------
  unsigned long now = millis();

  // Run PID every 100 ms
  if (now - lastPIDTime >= 100) {
    float dt = (now - lastPIDTime) / 1000.0; // seconds
    lastPIDTime = now;

    // Atomically read and reset pulse count
    noInterrupts();
    unsigned long count = pulseCount;
    pulseCount = 0;
    interrupts();

    // Calculate current RPM
    float trueSpeed = (count / (float)pulsesPerRev) * (60.0 / dt);

    // PID error
    float error = float(desiredSpeed - trueSpeed);
    integral += error * dt;
    float derivative = (error - lastError) / dt;
    lastError = error;

    // PID output
    float output = (Kp * error) + (Ki * integral) + (Kd * derivative);

    // Clamp output to PWM range
    if (output > 255) output = 255;
    if (output < 0) output = 0;

    // Apply to motor
    analogWrite(motorPWMPin, (int)output);

    // Debug info
    Serial.print("Target: "); Serial.print(desiredSpeed);
    Serial.print(" RPM | Actual: "); Serial.print(trueSpeed);
    Serial.print(" RPM | PWM: "); Serial.println(output);
  }

  //User code end loop() --------------------------------------------------------
}