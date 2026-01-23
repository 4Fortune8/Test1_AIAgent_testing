/*
 * ESP32 Hello World - Connection Verification
 * 
 * V1.03.00: Basic ESP32 setup before CAN implementation
 * User request: "lets begin ESP32 firmware development. start with a basic hello world check connections"
 * Plan: esp/thoughtprocesses/2026-01-22_hello-world-verify.md
 * 
 * V1.03.01: Added Arduino.h include for PlatformIO build
 * User request: "Fix directory structure, upload and test hello world"
 * 
 * Purpose:
 * - Verify ESP32 can be programmed via PlatformIO
 * - Test Serial communication at 115200 baud
 * - Confirm GPIO5/4 are accessible without boot issues
 * - Validate non-blocking loop patterns
 * 
 * Hardware:
 * - Board: ESP32-WROOM-32D on UPeasy devkit
 * - CAN Pins: GPIO5 (TX), GPIO4 (RX) - configured as INPUT for safety
 * - LED: GPIO2 (typical onboard LED)
 */

#include <Arduino.h>

#define LED_PIN 2      // Onboard LED (typical for ESP32)
#define CAN_TX_GPIO 5  // Future CAN TX pin
#define CAN_RX_GPIO 4  // Future CAN RX pin

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  delay(100);  // Allow serial to stabilize (acceptable in setup)
  
  // Print startup banner
  Serial.println("\n\n========================================");
  Serial.println("    ESP32 Hello World - V1.03.00");
  Serial.println("========================================");
  Serial.println("Board:     ESP32-WROOM-32D (UPeasy)");
  Serial.println("Framework: Arduino + PlatformIO");
  Serial.println("CAN Pins:  GPIO5 (TX), GPIO4 (RX)");
  Serial.println("========================================\n");
  
  // Configure CAN GPIO pins as INPUT (safe, non-interfering)
  pinMode(CAN_TX_GPIO, INPUT);
  pinMode(CAN_RX_GPIO, INPUT);
  Serial.println("[INIT] GPIO5 (CAN_TX) configured as INPUT");
  Serial.println("[INIT] GPIO4 (CAN_RX) configured as INPUT");
  
  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[INIT] GPIO2 (LED) configured as OUTPUT");
  
  Serial.println("\n[READY] Starting heartbeat loop...\n");
}

void loop() {
  static unsigned long lastHeartbeat = 0;
  static unsigned long heartbeatCount = 0;
  static bool ledState = false;
  
  unsigned long now = millis();
  
  // Non-blocking 1-second heartbeat (adheres to AGENTS.md: no blocking delays)
  if (now - lastHeartbeat >= 1000) {
    lastHeartbeat = now;
    heartbeatCount++;
    
    // Read GPIO states
    int gpio5_state = digitalRead(CAN_TX_GPIO);
    int gpio4_state = digitalRead(CAN_RX_GPIO);
    
    // Print heartbeat with GPIO status
    Serial.print("[HEARTBEAT #");
    Serial.print(heartbeatCount);
    Serial.print("] Uptime: ");
    Serial.print(now / 1000);
    Serial.print("s | GPIO5: ");
    Serial.print(gpio5_state ? "HIGH" : "LOW");
    Serial.print(" | GPIO4: ");
    Serial.println(gpio4_state ? "HIGH" : "LOW");
    
    // Toggle LED
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
