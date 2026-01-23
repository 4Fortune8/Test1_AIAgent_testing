# TWAI Driver Implementation - Phase 2

**Date:** 2026-01-22  
**Version:** V1.04.00 (step increment - new sub-feature)  
**User Request:** "Start PHASE 2 - TWAI DRIVER IMPLEMENTATION"

---

## Problem Statement

Implement ESP32 TWAI (Two-Wire Automotive Interface) CAN driver to enable communication between ESP32 and Raspberry Pi. This is the first step toward establishing PING/PONG protocol for CAN bus communication.

---

## Constraints

### Hard Rules (AGENTS.md)
- **NO blocking delays** in loop() - TWAI operations must be non-blocking
- **NO real-time logic on Pi** - ESP handles all real-time CAN operations
- **NO protocol changes** without explicit request
- **Must not interfere with Core 0** - CAN operations on Core 1 only (future)
- Must follow existing protocol specifications

### Hardware Constraints
- GPIO5 (TX), GPIO4 (RX) already allocated and verified
- SN65HVD230 transceiver in high-speed mode (RS grounded)
- 500 kbps bitrate (configured in platformio.ini)
- Must handle transceiver power-up states

### Protocol Constraints
- PING ID: 0x091 (from Pi)
- PONG ID: 0x111 (from ESP32)
- PONG payload: 4-byte uint32 sequence counter, little-endian
- Must follow 15 binding invariants in `protocol/can_invariants.md`

---

## Approach

### Phase 2.1: Basic TWAI Initialization (V1.04.00)
**Goal:** Initialize TWAI driver and verify it starts without errors

**Steps:**
1. Include ESP-IDF TWAI driver headers
2. Configure TWAI timing (500 kbps)
3. Configure TWAI filter (accept all for testing)
4. Configure GPIO5/4 pins
5. Install TWAI driver
6. Start TWAI driver
7. Add serial logging for status
8. Verify: "TWAI driver started" in serial output

**Testing:**
- ESP32 boots without panic
- Serial shows successful TWAI initialization
- No error flags in TWAI status

### Phase 2.2: Test Frame Transmission (V1.04.01)
**Goal:** Send test CAN frames to verify TX path

**Steps:**
1. Create test frame (ID=0x123, data="TEST")
2. Send frame every 2 seconds (non-blocking)
3. Log transmission status
4. Verify on Pi with `candump can0`

**Testing:**
- Pi receives frames with correct ID
- Transmission timing is consistent
- No blocking in loop()

### Phase 2.3: Frame Reception (V1.04.02)
**Goal:** Receive and log incoming CAN frames

**Steps:**
1. Check for received frames (non-blocking)
2. Read frame if available
3. Log frame ID and data
4. Verify with test frames from Pi

**Testing:**
- ESP32 receives frames sent from Pi
- Frame data logged correctly
- No blocking in loop()

### Phase 2.4: PING Detection (V1.04.03)
**Goal:** Detect PING frames (ID=0x091) and log

**Steps:**
1. Check received frame ID
2. If ID == 0x091, log "PING received"
3. Do NOT respond yet (PONG in Phase 2.5)
4. Continue processing other frames

**Testing:**
- Pi sends PING: `cansend can0 091#00000000`
- ESP32 logs "PING received"
- No PONG sent yet

### Phase 2.5: PONG Response (V1.04.04)
**Goal:** Respond to PING with PONG containing sequence counter

**Steps:**
1. Initialize static sequence counter (uint32_t, starts at 0)
2. When PING received, prepare PONG frame
3. PONG ID: 0x111
4. PONG data: sequence counter as 4-byte little-endian
5. Send PONG frame
6. Increment sequence counter (wraps at UINT32_MAX)
7. Log PONG transmission

**Testing:**
- Pi sends PING
- ESP32 responds with PONG (ID=0x111, DLC=4)
- Sequence counter increments correctly
- Little-endian byte order verified
- RTT measured on Pi

---

## Files to Modify

### Primary Implementation
- `esp32/src/main.cpp` - Add TWAI driver code

### Documentation
- `docs/agentcontext/checklist_can_ping_pong.md` - Mark phases complete
- `docs/agentcontext/version_log.md` - Add version entries
- `docs/agentcontext/CURRENT_STATE.md` - Update status

---

## Technical Details

### TWAI Configuration
```cpp
#include "driver/twai.h"

// Timing config for 500 kbps (from platformio.ini: CAN_BITRATE=500000)
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

// Filter config (accept all for now)
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

// GPIO config (from platformio.ini: CAN_TX_GPIO=5, CAN_RX_GPIO=4)
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    (gpio_num_t)CAN_TX_GPIO, 
    (gpio_num_t)CAN_RX_GPIO, 
    TWAI_MODE_NORMAL
);
```

### PONG Frame Structure
```cpp
uint32_t pong_sequence = 0;  // Static variable

// On PING received:
twai_message_t pong_msg;
pong_msg.identifier = 0x111;
pong_msg.data_length_code = 4;
pong_msg.flags = 0;  // Standard frame, data frame

// Little-endian encoding
pong_msg.data[0] = (pong_sequence >> 0) & 0xFF;
pong_msg.data[1] = (pong_sequence >> 8) & 0xFF;
pong_msg.data[2] = (pong_sequence >> 16) & 0xFF;
pong_msg.data[3] = (pong_sequence >> 24) & 0xFF;

twai_transmit(&pong_msg, pdMS_TO_TICKS(10));
pong_sequence++;
```

### Non-Blocking Pattern
```cpp
void loop() {
  static unsigned long lastHeartbeat = 0;
  static unsigned long lastTest = 0;
  unsigned long now = millis();
  
  // Check for received frames (non-blocking)
  twai_message_t rx_msg;
  if (twai_receive(&rx_msg, 0) == ESP_OK) {  // 0 = no wait
    handleCanFrame(&rx_msg);
  }
  
  // Periodic tasks (non-blocking timing)
  if (now - lastHeartbeat >= 1000) {
    // Heartbeat
  }
  
  if (now - lastTest >= 2000) {
    // Test frame
  }
}
```

---

## Risks

### Technical Risks
1. **TWAI driver conflicts with Arduino framework**
   - Mitigation: Use ESP-IDF TWAI driver directly, tested compatible
   
2. **Blocking calls in CAN operations**
   - Mitigation: Use timeout=0 for non-blocking, timeout=10ms max for TX
   
3. **GPIO conflicts or initialization order**
   - Mitigation: Initialize TWAI in setup(), after Serial
   
4. **Bus-off condition if no termination**
   - Mitigation: Hardware already verified with 120Ω termination
   
5. **Sequence counter endianness errors**
   - Mitigation: Explicit little-endian byte packing, verify on Pi

### Implementation Risks
1. **Breaking hello world functionality**
   - Mitigation: Keep heartbeat and LED logic, add CAN incrementally
   
2. **Serial output overflow**
   - Mitigation: Rate-limit CAN frame logging

---

## Testing Strategy

### Unit Testing (Per Phase)
- **Phase 2.1:** ESP boots, TWAI status OK
- **Phase 2.2:** Pi receives test frames
- **Phase 2.3:** ESP receives Pi test frames
- **Phase 2.4:** ESP detects PING frames
- **Phase 2.5:** Pi receives PONG with correct sequence

### Integration Testing
- PING/PONG round-trip works
- RTT measurement on Pi
- Sequence counter increments correctly
- No dropped frames under normal conditions

### Verification Commands
```bash
# On Pi - monitor CAN traffic
candump can0

# On Pi - send PING
cansend can0 091#00000000

# On Pi - verify PONG received
candump can0 | grep 111

# ESP32 - monitor serial
pio device monitor --baud 115200
```

---

## Success Criteria

### Phase 2.1 Complete When:
- [x] ESP32 boots without errors
- [x] Serial shows "TWAI driver started"
- [x] No error flags in TWAI status
- [x] Heartbeat continues working

### Phase 2.2 Complete When:
- [x] Pi receives test frames via candump
- [x] Frame ID and data correct
- [x] Timing consistent (every 2s)
- [x] No blocking detected

### Phase 2.3 Complete When:
- [x] ESP32 logs frames sent from Pi
- [x] Frame data decoded correctly
- [x] Multiple frames handled

### Phase 2.4 Complete When:
- [x] ESP32 detects PING (ID=0x091)
- [x] Serial shows "PING received"
- [x] No PONG sent yet

### Phase 2.5 Complete When:
- [x] ESP32 sends PONG in response to PING
- [x] PONG ID = 0x111, DLC = 4
- [x] Sequence counter increments
- [x] Little-endian encoding verified on Pi
- [x] RTT < 100ms

---

## Expected Outcome

After Phase 2 completion:
- ESP32 TWAI driver fully operational
- Basic PING/PONG protocol working
- Foundation for Phase 3 (Pi implementation)
- RTT measurements available
- Sequence counter tracking functional

---

## References

- **Protocol:** `protocol/can_id_map.md` (PING=0x091, PONG=0x111)
- **Invariants:** `protocol/can_invariants.md` (15 binding rules)
- **Roadmap:** `docs/agentcontext/roadmap_can_ping_pong.md` (Section 9)
- **Hardware:** `docs/agentcontext/hardware_wiring_guide.md`
- **Checklist:** `docs/agentcontext/checklist_can_ping_pong.md`

---

## Version Increments

- **V1.04.00** - TWAI initialization (Phase 2.1)
- **V1.04.01** - Test frame TX (Phase 2.2)
- **V1.04.02** - Frame RX (Phase 2.3)
- **V1.04.03** - PING detection (Phase 2.4)
- **V1.04.04** - PONG response (Phase 2.5)

Each phase will be committed separately with appropriate version tags and serial logging for verification.
