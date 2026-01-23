# CAN PING/PONG Implementation Checklist

**Roadmap Reference:** `docs/agentcontext/roadmap_can_ping_pong.md`  
**Date:** 2026-01-22  
**Status:** Phase 1 Complete - Ready for Phase 2 (TWAI Driver)

---

## Pre-Implementation (Human Approval Required) ✅ COMPLETE

- [x] **PONG Command ID Allocated** ✅ APPROVED
  - Decision: 0x001 under STATUS class (CAN ID=0x111)
  - Added to `protocol/can_id_map.md` with complete binary/hex/decimal table
  - Human approved: 2026-01-22

- [x] **PONG Payload Format** ✅ APPROVED
  - Decision: 4 bytes, uint32 sequence counter, little-endian
  - Enables duplicate detection, reboot detection, diagnostics
  - Human approved: 2026-01-22

- [x] **ESP32 GPIO Pins Confirmed** ✅ APPROVED (Updated 2026-01-22)
  - Board: ESP32-WROOM-32D on UPeasy devkit
  - Transceiver: SN65HVD230 (3.3V)
  - **CAN TX GPIO: GPIO5 (Pin 29)**
  - **CAN RX GPIO: GPIO4 (Pin 26)**
  - Rationale: Safe defaults, no boot conflicts
  - Human approved: 2026-01-22

- [x] **Pi SocketCAN Interface Confirmed** ✅ APPROVED (Updated 2026-01-22)
  - Board: Raspberry Pi 3B+
  - Transceiver: MCP2515 (SPI interface)
  - SPI Pins: MOSI=GPIO10, MISO=GPIO9, SCLK=GPIO11, CS=GPIO8, INT=GPIO25
  - Interface name: `can0`
  - Human approved: 2026-01-22

- [ ] **RTT Threshold Approved**
  - Target: <50ms average
  - Maximum acceptable: 100ms
  - Status: ✅ APPROVED (2026-01-22)

**Hardware Wiring Status:**
- [x] SN65HVD230 wired to ESP32 GPIO5/4 ✅ **COMPLETE**
- [x] SN65HVD230 RS pin grounded (high-speed mode) ✅ **COMPLETE**
- [x] 120Ω termination resistors installed at both ends ✅ **COMPLETE**
- [x] MCP2515 `/boot/firmware/config.txt` overlay configured (interrupt=25) ✅ **COMPLETE**
- [x] Shared ground between ESP32 and Pi ✅ **COMPLETE**
- [x] Pi `can0` interface UP and operational ✅ **COMPLETE**

**New Protocol Documents Created:**
- [x] `protocol/can_id_map.md` - Updated with complete ID reference table
- [x] `protocol/can_invariants.md` - 15 binding invariants documented

---

## Phase 0: Hello World (ESP32 Verification) ✅ COMPLETE (V1.03.01)

- [x] **ESP32 Hello World Implementation**
  - Planning doc created: `esp/thoughtprocesses/2026-01-22_hello-world-verify.md`
  - Source code: `esp32/src/main.cpp` (with `#include <Arduino.h>`)
  - Build successful: 20.6% Flash, 6.6% RAM
  - Upload successful: ESP32-D0WD-V3 (rev v3.1), MAC: 88:13:bf:07:ae:e4
  - Serial output verified: Heartbeat every 1s at 115200 baud
  - GPIO5: HIGH, GPIO4: LOW confirmed
  - LED blinking on GPIO2 confirmed
  - Non-blocking loop pattern verified
  - Version tag: `V1.03.00` (initial), `V1.03.01` (with Arduino.h)

---

## Planning Documents (MUST CREATE FIRST)

- [ ] `esp/thoughtprocesses/2026-01-22_can-bringup-esp.md` created
  - Contains: pin assignments, TWAI config, core affinity, error handling, test strategy
  - References: `protocol/can_id_map.md` for IDs, `protocol/can_invariants.md` for rules
  
- [ ] `pi/thoughtprocesses/2026-01-22_can-bringup-pi.md` created
  - Contains: interface setup, library choice, PING/PONG logic, error handling
  - Includes: sequence counter validation, single-node assertion
  
- [ ] `protocol/thoughtprocesses/2026-01-22_ping-pong-spec.md` created
  - Contains: PING/PONG message specs, sequence diagrams, validation criteria
  - Specifies: 4-byte counter behavior, little-endian encoding, node ID filtering

- [ ] Planning documents reviewed by human: _______________

---

## Implementation Phase 1: ESP32 TWAI Initialization ✅ COMPLETE (V1.04.00)

- [x] **Phase 3.1: TWAI Driver Init**
  - ESP boots without crash ✅
  - Serial log shows "TWAI driver started" ✅
  - TWAI status: RUNNING ✅
  - No error flags in status (Err:0) ✅
  - Heartbeat continues working ✅
  - LED blinking confirmed ✅
  - Version tag added: `V1.04.00` ✅

---

## Implementation Phase 2: ESP32 RX/TX Testing

- [ ] **Phase 3.2: CAN RX Task on Core 1**
  - RX task created and pinned to Core 1
  - Task receives test messages
  - Control loop (Core 0) unaffected
  - Version tag added: `V1.00.02`

- [ ] **Phase 3.3: PING Handler (Logging Only)**
  - ESP logs "PING received"
  - Does not transmit yet
  - Control loop unaffected
  - Version tag added: `V1.00.03`

---

## Implementation Phase 3: Pi SocketCAN Bring-Up

- [ ] **Phase 4.1: SocketCAN Configuration**
  - `can0` interface UP
  - Bitrate 500 kbps confirmed
  - No errors in `dmesg`

- [ ] **Phase 4.2: Python CAN Library**
  - `python-can` installed
  - Test script runs without errors
  - Can send/receive test frames

- [ ] **Phase 4.3: PING Transmission**
  - Pi sends PING at 1 Hz
  - `candump` shows ID=0x091
  - ESP logs match
  - Version tag added: `V1.00.04`

---

## Implementation Phase 4: PONG Response

- [ ] **Phase 3.4: ESP TX Implementation**
  - ESP transmits PONG after PING with 4-byte sequence counter
  - Counter starts at 0, increments per PONG (wraps at UINT32_MAX)
  - Little-endian encoding verified
  - Pi receives PONG (ID=0x111, DLC=4)
  - RTT <50ms measured
  - Version tag added: `V1.00.05`

- [ ] **Phase 4.4: Pi PONG Reception**
  - Pi calculates and logs RTT
  - Pi parses sequence counter (little-endian)
  - Pi validates DLC==4, Node ID==0x1
  - Pi detects duplicates (same seq), gaps (seq jumps), reboots (seq reset)
  - Single-node assertion: Pi logs error if multiple PONGs received
  - Success rate tracked
  - Version tag added: `V1.00.06`

---

## Implementation Phase 5: Error Handling

- [ ] **Phase 3.5: ESP Robustness**
  - Bus-off recovery implemented
  - TX timeout handling
  - No blocking calls verified
  - Version tag added: `V1.00.07`

- [ ] **Phase 4.5: Pi Robustness**
  - Interface error handling
  - Statistics tracking
  - Graceful shutdown
  - Version tag added: `V1.00.08`

---

## Validation Tests (All Must Pass)

- [ ] **Test 1: Single PING/PONG**
  - RTT <100ms, no errors

- [ ] **Test 2: Continuous 1 Hz (1 minute)**
  - >95% success rate, no crashes

- [ ] **Test 3: High-Rate 10 Hz (10 seconds)**
  - >90% success rate, control loop unaffected

- [ ] **Test 4: ESP Restart**
  - Pi recovers automatically

- [ ] **Test 5: CAN Interface Restart**
  - Pi logs error, recovers

- [ ] **Test 6: Cable Disconnect**
  - Both nodes recover

- [ ] **Test 7: Malformed Messages**
  - ESP ignores gracefully

- [ ] **Test 8: 24-Hour Stability**
  - >99% success rate, no leaks, no crashes

---

## Exit Criteria (ALL Required)

### Communication Reliability
- [ ] PING/PONG >99% success over 24 hours
- [ ] RTT <50ms average (p99 <100ms)
- [ ] No memory leaks
- [ ] No crashes/panics in 24 hours

### Error Handling
- [ ] ESP recovers from bus-off
- [ ] Pi recovers from interface restart
- [ ] Cable disconnect/reconnect handled
- [ ] Invalid messages ignored

### Core Isolation
- [ ] Core 0 uninterrupted during CAN
- [ ] CAN task Core 1 verified
- [ ] No blocking calls audited

### Documentation
- [ ] Planning docs complete
- [ ] Code comments with version tags
- [ ] `version_log.md` updated
- [ ] Failure recovery documented

### Testing
- [ ] All Section 5.2 tests passed
- [ ] Results logged and archived
- [ ] Known issues documented

### Protocol
- [ ] PONG ID in `can_id_map.md` ✅ (0x111 already added)
- [ ] CAN ID verified (scope/analyzer)
- [ ] Payloads match spec exactly (4-byte counter, little-endian)
- [ ] Sequence counter behavior verified
- [ ] Single-node response verified
- [ ] Node ID filtering tested
- [ ] CAN invariants followed (see `protocol/can_invariants.md`)

---

## Post-Implementation

- [ ] **Version Log Updated**
  - Entry added to `docs/agentcontext/version_log.md`
  - Includes: version, date, summary, files, planning docs

- [ ] **Human Review Completed**
  - Functionality demonstrated
  - Test results reviewed
  - Approved for next phase: _______________

---

## Notes / Issues Encountered

```
[Space for implementation notes]







```

---

**DO NOT proceed to motor control commands until ALL exit criteria are met.**

Refer to `roadmap_can_ping_pong.md` for detailed instructions on each phase.
