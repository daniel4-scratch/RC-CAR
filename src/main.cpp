#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>

#define LED_PIN LED_BUILTIN
#define THROTTLE_PIN 5
#define THROTTLE_FORWARD 4
#define STEERING_PIN 6
#define STEERING_FORWARD 7

unsigned long lastBlink = 0;
bool ledState = false;
int blinkInterval = 500;

Servo throttleOut;
Servo steeringOut;

unsigned int lastThrottleUs = 1500;
unsigned int lastSteeringUs = 1500;
volatile unsigned int throttleUs = 1500; // updated by ISR
volatile unsigned int steeringUs = 1500; // updated by ISR

// brake lights (persist between loops, controlled by UDP)
volatile bool breakLights = true;

// ISR state capture
volatile unsigned long throttleRiseMicros = 0;
volatile unsigned long steeringRiseMicros = 0;

// Interrupt Service Routines to measure RC PWM pulse width non-blocking
void IRAM_ATTR throttleISR() {
  int level = digitalRead(THROTTLE_PIN);
  if (level == HIGH) {
    throttleRiseMicros = micros();
  } else {
    unsigned long width = micros() - throttleRiseMicros;
    if (width >= 800 && width <= 2400) { // sanity bounds
      throttleUs = (unsigned int)constrain(width, 1000UL, 2000UL);
    }
  }
}

void IRAM_ATTR steeringISR() {
  int level = digitalRead(STEERING_PIN);
  if (level == HIGH) {
    steeringRiseMicros = micros();
  } else {
    unsigned long width = micros() - steeringRiseMicros;
    if (width >= 800 && width <= 2400) {
      steeringUs = (unsigned int)constrain(width, 1000UL, 2000UL);
    }
  }
}


void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial test: ESP32-S2 booted");
  pinMode(LED_PIN, OUTPUT);
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(STEERING_PIN, INPUT);
  pinMode(12, OUTPUT); //break
  pinMode(11, OUTPUT); //break
  pinMode(21, OUTPUT); //headlight
  pinMode(20, OUTPUT); //headlight

  // int servos
  throttleOut.attach(THROTTLE_FORWARD);
  steeringOut.attach(STEERING_FORWARD);
  throttleOut.writeMicroseconds(lastThrottleUs);
  steeringOut.writeMicroseconds(lastSteeringUs);

  // Attach interrupts for non-blocking pulse measurement
  attachInterrupt(THROTTLE_PIN, throttleISR, CHANGE);
  attachInterrupt(STEERING_PIN, steeringISR, CHANGE);
}

void loop() {
  unsigned int currentThrottle = throttleUs;
  unsigned int currentSteering = steeringUs;

  // Simple smoothing: only update servos if change exceeds threshold to reduce visible jitter
  const int jitterThreshold = 3; // microseconds
  if (abs((int)currentThrottle - (int)lastThrottleUs) > jitterThreshold) {
    throttleOut.writeMicroseconds((int)currentThrottle);
    lastThrottleUs = currentThrottle;
  }
  if (abs((int)currentSteering - (int)lastSteeringUs) > jitterThreshold) {
    steeringOut.writeMicroseconds((int)currentSteering);
    lastSteeringUs = currentSteering;
  }

  // LEDs
  if (currentThrottle < 1450) {
    // reverse
    blinkInterval = 200;
    if(breakLights){
      digitalWrite(12, HIGH);
      digitalWrite(11, HIGH);
    }else{
      digitalWrite(12, LOW);
      digitalWrite(11, LOW);
    }
  } else if (currentThrottle > 1550) {
    // forward
    blinkInterval = map(currentThrottle, 1550, 2000, 150, 30);
    blinkInterval = constrain(blinkInterval, 30, 150);
  } else {
    // neutral
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(12, LOW);
    digitalWrite(11, LOW);
    return;
  }

  // non blocked blink
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlink >= blinkInterval) {
    lastBlink = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
