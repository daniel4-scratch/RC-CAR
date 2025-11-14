#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>

// WiFi
#include <WiFi.h>
#include <WiFiUdp.h>
const char* ssid = "RC-CAR";
const char* password = "123bingus";
WiFiUDP udp;
unsigned int port = 4210;

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


void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Serial test: ESP32-S2 booted");
  pinMode(LED_PIN, OUTPUT);
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(STEERING_PIN, INPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);

  // int servos
  throttleOut.attach(THROTTLE_FORWARD);
  steeringOut.attach(STEERING_FORWARD);
  throttleOut.writeMicroseconds(lastThrottleUs);
  steeringOut.writeMicroseconds(lastSteeringUs);

  // init wifi and udp
  WiFi.softAP(ssid, password);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  udp.begin(port);
  Serial.print("UDP Port: ");
  Serial.println(port);
}

void loop() {
  // throttle
  unsigned long throttle = pulseIn(THROTTLE_PIN, HIGH, 25000);
  if (throttle == 0) {
    throttle = lastThrottleUs;
  }
  throttle = constrain(throttle, 1000UL, 2000UL);
  lastThrottleUs = throttle;

  // update throttle
  throttleOut.writeMicroseconds((int)throttle);

  // steering
  unsigned long steering = pulseIn(STEERING_PIN, HIGH, 25000);
  if (steering == 0) {
    steering = lastSteeringUs;
  }
  steering = constrain(steering, 1000UL, 2000UL);
  lastSteeringUs = steering;
  // update servo
  steeringOut.writeMicroseconds((int)steering);

  // LEDs
  if (throttle < 1450) {
    // reverse
    blinkInterval = 200;
    digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);
  } else if (throttle > 1550) {
    // forward
    blinkInterval = map(throttle, 1550, 2000, 150, 30);
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

  // WiFi
  int packetSize = udp.parsePacket();
  if (packetSize){
    char msg[20];
    int len = udp.read(msg,20);
    if (len > 0) msg[len] = 0;
    Serial.print("Received: ");
    Serial.println(msg);
  }
}
