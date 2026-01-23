/*
 * ESP32 CAN PING/PONG - TWAI Driver Implementation
 * 
 * V1.03.00: Basic ESP32 setup before CAN implementation
 * User request: "lets begin ESP32 firmware development. start with a basic hello world check connections"
 * Plan: esp/thoughtprocesses/2026-01-22_hello-world-verify.md
 * 
 * V1.03.01: Added Arduino.h include for PlatformIO build
 * User request: "Fix directory structure, upload and test hello world"
 * 
 * V1.04.00: TWAI driver initialization
 * User request: "Start PHASE 2 - TWAI DRIVER IMPLEMENTATION"
 * Plan: esp/thoughtprocesses/2026-01-22_twai-driver-implementation.md
 * 
 * V1.04.01: Test frame transmission
 * User request: "lets begin development of stages for Phase 2.2: Test frame transmission"
 * Plan: esp/thoughtprocesses/2026-01-22_twai-driver-implementation.md (Phase 2.2)
 * 
 * Purpose:
 * - Implement ESP32 TWAI (CAN) driver
 * - Enable PING/PONG protocol communication with Raspberry Pi
 * - Maintain non-blocking operation
 * - Validate CAN bus communication at 500 kbps
 * 
 * Hardware:
 * - Board: ESP32-WROOM-32D on UPeasy devkit
 * - CAN Pins: GPIO5 (TX), GPIO4 (RX) → SN65HVD230 transceiver
 * - CAN Bitrate: 500 kbps
 * - LED: GPIO2 (typical onboard LED)
 */

#include <Arduino.h>
#include "driver/twai.h"

#define LED_PIN 2      // Onboard LED (typical for ESP32)
#define CAN_TX_GPIO 5  // CAN TX pin (also defined in platformio.ini)
#define CAN_RX_GPIO 4  // CAN RX pin (also defined in platformio.ini)

// V1.04.00: TWAI driver initialization
bool twaiInitialized = false;

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  delay(100);  // Allow serial to stabilize (acceptable in setup)
  
  // Print startup banner
  Serial.println("\n\n========================================");
  Serial.println("    ESP32 CAN PING/PONG - V1.04.01");
  Serial.println("========================================");
  Serial.println("Board:     ESP32-WROOM-32D (UPeasy)");
  Serial.println("Framework: Arduino + PlatformIO");
  Serial.println("CAN Pins:  GPIO5 (TX), GPIO4 (RX)");
  Serial.println("CAN Mode:  TWAI Driver (500 kbps)");
  Serial.println("Phase:     2.2 - Test Frame TX");
  Serial.println("========================================\n");
  
  // V1.04.00: Initialize TWAI (CAN) driver
  Serial.println("[TWAI] Initializing TWAI driver...");
  
  // General configuration: 500 kbps, normal mode
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    (gpio_num_t)CAN_TX_GPIO, 
    (gpio_num_t)CAN_RX_GPIO, 
    TWAI_MODE_NORMAL
  );
  g_config.tx_queue_len = 10;  // TX queue size
  g_config.rx_queue_len = 10;  // RX queue size
  
  // Timing configuration: 500 kbps
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  
  // Filter configuration: Accept all messages for now
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  
  // Install TWAI driver
  esp_err_t result = twai_driver_install(&g_config, &t_config, &f_config);
  if (result == ESP_OK) {
    Serial.println("[TWAI] Driver installed successfully");
    
    // Start TWAI driver
    result = twai_start();
    if (result == ESP_OK) {
      Serial.println("[TWAI] Driver started successfully");
      twaiInitialized = true;
      
      // Get and print TWAI status
      twai_status_info_t status;
      twai_get_status_info(&status);
      Serial.printf("[TWAI] State: %s\n", 
        status.state == TWAI_STATE_RUNNING ? "RUNNING" :
        status.state == TWAI_STATE_BUS_OFF ? "BUS_OFF" :
        status.state == TWAI_STATE_RECOVERING ? "RECOVERING" :
        status.state == TWAI_STATE_STOPPED ? "STOPPED" : "UNKNOWN");
      Serial.printf("[TWAI] TX Queue: %d, RX Queue: %d\n", 
        status.msgs_to_tx, status.msgs_to_rx);
    } else {
      Serial.printf("[TWAI] ERROR: Failed to start driver (0x%X)\n", result);
    }
  } else {
    Serial.printf("[TWAI] ERROR: Failed to install driver (0x%X)\n", result);
  }
  
  Serial.println();
  
  // Configure CAN GPIO pins as INPUT (safe, non-interfering)
  pinMode(CAN_TX_GPIO, INPUT);
  pinMode(CAN_RX_GPIO, INPUT);
  Serial.println("[INIT] GPIO5 (CAN_TX) configured as INPUT");
  Serial.println("[INIT] GPIO4 (CAN_RX) configured as INPUT");
  
  // Configure LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[INIT] GPIO2 (LED) configured as OUTPUT");
  
  if (twaiInitialized) {
    Serial.println("\n[READY] TWAI driver operational - Starting main loop...\n");
  } else {
    Serial.println("\n[WARNING] TWAI driver failed - Running in test mode...\n");
  }
}

void loop() {
  static unsigned long lastHeartbeat = 0;
  static unsigned long lastTestFrame = 0;  // V1.04.01: Test frame timing
  static unsigned long heartbeatCount = 0;
  static unsigned long testFrameCount = 0;  // V1.04.01: Test frame counter
  static bool ledState = false;
  
  unsigned long now = millis();
  
  // V1.04.01: Send test CAN frame every 2 seconds (non-blocking)
  if (twaiInitialized && (now - lastTestFrame >= 2000)) {
    lastTestFrame = now;
    
    // V1.04.01: Check TWAI status before transmitting
    twai_status_info_t status;
    twai_get_status_info(&status);
    
    // V1.04.01: Recover from BUS_OFF state
    if (status.state == TWAI_STATE_BUS_OFF) {
      Serial.println("[TWAI] BUS_OFF detected - initiating recovery...");
      twai_initiate_recovery();
      // Wait for recovery to complete (non-blocking check in next iteration)
      return;  // Skip this loop iteration
    }
    
    // V1.04.01: Restart if in STOPPED or RECOVERING state
    if (status.state == TWAI_STATE_STOPPED || status.state == TWAI_STATE_RECOVERING) {
      Serial.println("[TWAI] Restarting driver...");
      esp_err_t result = twai_start();
      if (result == ESP_OK) {
        Serial.println("[TWAI] Driver restarted successfully");
      } else {
        Serial.printf("[TWAI] ERROR: Failed to restart (0x%X)\n", result);
      }
      return;  // Skip this loop iteration
    }
    
    // Only transmit if in RUNNING state
    if (status.state == TWAI_STATE_RUNNING) {
      testFrameCount++;
      
      // Create test frame (ID=0x123, 8 bytes: "TEST" + counter)
      twai_message_t test_msg;
      test_msg.identifier = 0x123;
      test_msg.data_length_code = 8;
      test_msg.flags = 0;  // Standard frame, data frame
      
      // Fill data: "TEST" (4 bytes) + counter (4 bytes, little-endian)
      test_msg.data[0] = 'T';
      test_msg.data[1] = 'E';
      test_msg.data[2] = 'S';
      test_msg.data[3] = 'T';
      test_msg.data[4] = (testFrameCount >> 0) & 0xFF;
      test_msg.data[5] = (testFrameCount >> 8) & 0xFF;
      test_msg.data[6] = (testFrameCount >> 16) & 0xFF;
      test_msg.data[7] = (testFrameCount >> 24) & 0xFF;
      
      // Transmit with 10ms timeout (non-blocking)
      esp_err_t result = twai_transmit(&test_msg, pdMS_TO_TICKS(10));
      if (result == ESP_OK) {
        Serial.printf("[CAN_TX] Test frame #%lu sent (ID=0x123)\n", testFrameCount);
      } else if (result == ESP_ERR_TIMEOUT) {
        Serial.println("[CAN_TX] ERROR: TX timeout");
      } else if (result == ESP_FAIL) {
        Serial.println("[CAN_TX] ERROR: TX queue full");
      } else {
        Serial.printf("[CAN_TX] ERROR: TX failed (0x%X)\n", result);
      }
    } else {
      Serial.printf("[TWAI] Waiting for RUNNING state (current: %d)\n", status.state);
    }
  }
  
  // V1.04.00: Non-blocking heartbeat with TWAI status
  if (now - lastHeartbeat >= 1000) {
    lastHeartbeat = now;
    heartbeatCount++;
    
    // Print heartbeat
    Serial.print("[HEARTBEAT #");
    Serial.print(heartbeatCount);
    Serial.print("] Uptime: ");
    Serial.print(now / 1000);
    Serial.print("s");
    
    // Print TWAI status if initialized
    if (twaiInitialized) {
      twai_status_info_t status;
      if (twai_get_status_info(&status) == ESP_OK) {
        Serial.print(" | TWAI: ");
        Serial.print(status.state == TWAI_STATE_RUNNING ? "RUN" :
                    status.state == TWAI_STATE_BUS_OFF ? "BUS_OFF" :
                    status.state == TWAI_STATE_RECOVERING ? "RECOVER" : "STOP");
        Serial.print(" | TX:");
        Serial.print(status.msgs_to_tx);
        Serial.print(" RX:");
        Serial.print(status.msgs_to_rx);
        Serial.print(" Err:");
        Serial.print(status.tx_error_counter + status.rx_error_counter);
      }
    }
    
    Serial.println();
    
    // Toggle LED
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
