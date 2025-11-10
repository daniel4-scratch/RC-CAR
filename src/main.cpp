#include <Arduino.h>
#include <ESP32Servo.h>

#define LED_PIN LED_BUILTIN
#define THROTTLE_PIN 5
#define THROTTLE_FORWARD 4
#define STEERING_PIN 6
#define STEERING_FORWARD 7

unsigned long lastBlink = 0;
bool ledState = false;
int blinkInterval = 500;

// Servo/ESC outputs driven by hardware timer (non-blocking)
Servo throttleOut;
Servo steeringOut;

// Last known valid pulse values (us)
unsigned int lastThrottleUs = 1500;
unsigned int lastSteeringUs = 1500;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(STEERING_PIN, INPUT);
  pinMode(21, OUTPUT);
  Serial.begin(115200);

  // Attach servo/ESC outputs (50 Hz handled by library)
  throttleOut.attach(THROTTLE_FORWARD);
  steeringOut.attach(STEERING_FORWARD);

  // Initialize outputs to neutral so ESC/servo arm correctly
  throttleOut.writeMicroseconds(lastThrottleUs);
  steeringOut.writeMicroseconds(lastSteeringUs);
}

void loop() {
  // --- Read throttle ---
  unsigned long throttle = pulseIn(THROTTLE_PIN, HIGH, 25000);
  if (throttle == 0) {
    // keep last value on timeout
    throttle = lastThrottleUs;
  }
  // Sanitize and store
  throttle = constrain(throttle, 1000UL, 2000UL);
  lastThrottleUs = throttle;

  // --- Update ESC output (non-blocking) ---
  throttleOut.writeMicroseconds((int)throttle);

  // --- Read steering ---
  unsigned long steering = pulseIn(STEERING_PIN, HIGH, 25000);
  if (steering == 0) {
    // keep last value on timeout
    steering = lastSteeringUs;
  }
  // Sanitize and store
  steering = constrain(steering, 1000UL, 2000UL);
  lastSteeringUs = steering;

  // --- Update steering servo (non-blocking) ---
  steeringOut.writeMicroseconds((int)steering);

  // --- LED behavior logic ---
  if (throttle < 1450) {
    // Reverse → fixed flash rate
    blinkInterval = 200;
    digitalWrite(21, HIGH);

  } else if (throttle > 1550) {
    // Forward → flash faster depending on throttle
    blinkInterval = map(throttle, 1550, 2000, 150, 30);
    blinkInterval = constrain(blinkInterval, 30, 150);

  } else {
    // Brake/neutral → solid light
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(21, LOW);
    return; // skip blinking logic this frame
  }

  // --- Non-blocking blink ---
  unsigned long currentMillis = millis();
  if (currentMillis - lastBlink >= blinkInterval) {
    lastBlink = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
