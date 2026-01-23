# CAN Protocol Complete Specification - Motor Control System

**Date:** 2026-01-22  
**Version:** V2.00.00 (Major architecture change - requires approval when implementing)  
**Status:** PLANNING DOCUMENT  
**Parent Plan:** docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md

---

## Overview

This document defines the complete CAN protocol for the ESP32/Pi motor control system. It extends the existing PING/PONG protocol with comprehensive motor control, status reporting, and safety features.

---

## Message ID Allocation

### Command Messages (Pi → ESP32)
| ID (Hex) | ID (Dec) | Name | DLC | Priority | Description |
|----------|----------|------|-----|----------|-------------|
| 0x080 | 128 | SYSTEM_RESET | 0 | Critical | Software reset ESP32 |
| 0x081 | 129 | MOTOR_ENABLE | 1 | Critical | Enable/disable motors |
| 0x082 | 130 | SET_VELOCITY | 8 | High | Set target velocities (both motors) |
| 0x083 | 131 | SET_POSITION | 8 | High | Set target positions (both motors) |
| 0x084 | 132 | SET_PID_GAINS | 12* | Low | Update PID parameters |
| 0x085 | 133 | EMERGENCY_STOP | 0 | Critical | Immediate stop, enter ESTOP state |
| 0x086 | 134 | RESET_POSITION | 0 | Medium | Zero encoder positions |
| 0x087 | 135 | SET_ACCEL_LIMIT | 4 | Low | Set acceleration limit |
| 0x088 | 136 | SET_CURRENT_LIMIT | 4 | Medium | Set max current limit |
| 0x089 | 137 | CLEAR_FAULT | 0 | Medium | Clear fault state, return to IDLE |
| 0x08A | 138 | SET_MODE | 1 | Medium | Switch control mode (velocity/position) |
| 0x08B | 139 | TRAJECTORY_POINT | 8 | High | Single trajectory setpoint |
| 0x08C | 140 | TRAJECTORY_START | 0 | Medium | Begin trajectory execution |
| 0x08D | 141 | TRAJECTORY_STOP | 0 | Medium | Stop trajectory, hold position |
| 0x08E | 142 | SET_HOME_POSITION | 8 | Low | Define home position |
| 0x08F | 143 | GO_TO_HOME | 0 | Medium | Move to home position |
| 0x090 | 144 | REQUEST_STATUS | 1 | Low | Request specific status message |
| 0x091 | 145 | PING | 4 | Low | Ping with timestamp (existing) |

*Note: Multi-frame messages use CAN 2.0A standard frames with sequence numbering

### Status Messages (ESP32 → Pi)
| ID (Hex) | ID (Dec) | Name | DLC | Rate | Description |
|----------|----------|------|-----|------|-------------|
| 0x100 | 256 | HEARTBEAT | 1 | 10 Hz | System state, always transmitted |
| 0x101 | 257 | MOTOR_STATUS | 8 | 10 Hz | Enable flags, error counters |
| 0x102 | 258 | POSITION_REPORT | 8 | 50 Hz | Current encoder positions |
| 0x103 | 259 | VELOCITY_REPORT | 8 | 50 Hz | Current measured velocities |
| 0x104 | 260 | FAULT_REPORT | 4 | On change | Fault flags and codes |
| 0x105 | 261 | PID_DEBUG | 12* | 1 Hz | P, I, D terms for debugging |
| 0x106 | 262 | CURRENT_REPORT | 8 | 10 Hz | Motor current measurements |
| 0x107 | 263 | TEMPERATURE_REPORT | 4 | 1 Hz | Driver temperature sensors |
| 0x108 | 264 | BUFFER_STATUS | 4 | 10 Hz | Trajectory buffer fill level |
| 0x109 | 265 | LIMIT_SWITCH_STATUS | 1 | On change | Limit switch states |
| 0x10A | 266 | ERROR_LOG | 8 | On event | Timestamped error events |
| 0x10B | 267 | PERFORMANCE_METRICS | 8 | 1 Hz | Loop timing, CPU load |
| 0x111 | 273 | PONG | 4 | On PING | Response to PING (existing) |

### Broadcast Messages (Any → Any)
| ID (Hex) | ID (Dec) | Name | DLC | Description |
|----------|----------|------|-----|-------------|
| 0x200 | 512 | GLOBAL_ESTOP | 0 | Network-wide emergency stop |
| 0x201 | 513 | WATCHDOG_ALERT | 1 | Watchdog timeout warning |
| 0x202 | 514 | SYNC_PULSE | 4 | Time synchronization |

---

## Message Definitions

### 0x080: SYSTEM_RESET
**Direction:** Pi → ESP32  
**DLC:** 0  
**Description:** Soft reset ESP32, equivalent to pressing reset button  
**Payload:** None  
**Response:** ESP32 reboots, sends HEARTBEAT with state=INIT  
**Safety:** Does NOT clear ESTOP condition  

### 0x081: MOTOR_ENABLE
**Direction:** Pi → ESP32  
**DLC:** 1  
**Payload:**
```
Byte 0: Enable flags
  Bit 0: Motor 1 enable (0=disable, 1=enable)
  Bit 1: Motor 2 enable
  Bit 2: Brake 1 (if equipped)
  Bit 3: Brake 2 (if equipped)
  Bits 4-7: Reserved
```
**Response:** MOTOR_STATUS within 10ms  
**Safety:** 
- Cannot enable if in FAULT or ESTOP state
- Disabling motors immediately sets PWM to 0

### 0x082: SET_VELOCITY
**Direction:** Pi → ESP32  
**DLC:** 8  
**Payload:**
```
Bytes 0-3: Motor 1 velocity (float32, RPM, little-endian)
Bytes 4-7: Motor 2 velocity (float32, RPM, little-endian)
```
**Range:** -120.0 to +120.0 RPM  
**Response:** VELOCITY_REPORT within 20ms  
**Safety:**
- Velocity clamped to configured limits
- Acceleration limiting applied automatically
- Ignored if motors not enabled

### 0x083: SET_POSITION
**Direction:** Pi → ESP32  
**DLC:** 8  
**Payload:**
```
Bytes 0-3: Motor 1 target position (int32, encoder counts, little-endian)
Bytes 4-7: Motor 2 target position (int32, encoder counts, little-endian)
```
**Range:** -2147483648 to +2147483647 counts  
**Response:** POSITION_REPORT  
**Behavior:**
- Generates trapezoidal velocity profile
- Uses configured max velocity and acceleration
- Reports completion via MOTOR_STATUS

### 0x084: SET_PID_GAINS
**Direction:** Pi → ESP32  
**DLC:** 12 (requires 2 frames)  
**Payload (Frame 1):**
```
Byte 0: Sequence number (0)
Byte 1: Motor select (0=Motor1, 1=Motor2, 2=Both)
Bytes 2-5: Kp (float32)
Bytes 6-7: (Ki bytes 0-1)
```
**Payload (Frame 2):**
```
Byte 0: Sequence number (1)
Bytes 1-2: (Ki bytes 2-3)
Bytes 3-6: Kd (float32)
Byte 7: Reserved
```
**Response:** Acknowledgment in MOTOR_STATUS  
**Safety:** New gains applied at loop start to avoid discontinuity

### 0x085: EMERGENCY_STOP
**Direction:** Pi → ESP32 (or broadcast)  
**DLC:** 0  
**Payload:** None  
**Response:** Immediate state transition to ESTOP  
**Behavior:**
- PWM → 0 within 500μs
- Brakes engaged (if equipped)
- All trajectory buffers cleared
- Requires CLEAR_FAULT + manual intervention to recover

### 0x086: RESET_POSITION
**Direction:** Pi → ESP32  
**DLC:** 0  
**Payload:** None  
**Effect:** Set both encoder positions to 0  
**Safety:** Only allowed when motors disabled

### 0x087: SET_ACCEL_LIMIT
**Direction:** Pi → ESP32  
**DLC:** 4  
**Payload:**
```
Bytes 0-3: Acceleration limit (float32, RPM/s, little-endian)
```
**Range:** 1.0 to 10000.0 RPM/s  
**Default:** 1000.0 RPM/s

### 0x088: SET_CURRENT_LIMIT
**Direction:** Pi → ESP32  
**DLC:** 4  
**Payload:**
```
Bytes 0-3: Current limit (float32, Amps, little-endian)
```
**Range:** 0.1 to 10.0 A  
**Safety:** Exceeding this limit triggers FAULT state

### 0x089: CLEAR_FAULT
**Direction:** Pi → ESP32  
**DLC:** 0  
**Payload:** None  
**Behavior:**
- Transitions from FAULT → IDLE
- Does NOT work for ESTOP (requires power cycle or physical reset)
- Clears error counters

### 0x08A: SET_MODE
**Direction:** Pi → ESP32  
**DLC:** 1  
**Payload:**
```
Byte 0: Control mode
  0x00 = VELOCITY mode (default)
  0x01 = POSITION mode
  0x02 = TRAJECTORY mode
  0x03 = TORQUE mode (future)
```
**Safety:** Mode change only allowed when velocity = 0

### 0x08B: TRAJECTORY_POINT
**Direction:** Pi → ESP32  
**DLC:** 8  
**Payload:**
```
Byte 0: Sequence number (rolling 0-255)
Bytes 1-4: Timestamp (uint32, milliseconds from trajectory start)
Bytes 5-6: Velocity 1 (int16, RPM * 10)
Bytes 7: Velocity 2 (int8, delta from velocity 1, RPM * 10)
```
**Buffer:** 50 point circular buffer  
**Rate:** Up to 100 Hz streaming

### 0x08C: TRAJECTORY_START
**Direction:** Pi → ESP32  
**DLC:** 0  
**Effect:** Begin executing buffered trajectory  
**Safety:** Requires at least 10 points in buffer

### 0x08D: TRAJECTORY_STOP
**Direction:** Pi → ESP32  
**DLC:** 0  
**Effect:** Stop trajectory execution, hold current position

### 0x08E: SET_HOME_POSITION
**Direction:** Pi → ESP32  
**DLC:** 8  
**Payload:**
```
Bytes 0-3: Home position Motor 1 (int32, counts)
Bytes 4-7: Home position Motor 2 (int32, counts)
```

### 0x08F: GO_TO_HOME
**Direction:** Pi → ESP32  
**DLC:** 0  
**Effect:** Move to previously defined home position

### 0x090: REQUEST_STATUS
**Direction:** Pi → ESP32  
**DLC:** 1  
**Payload:**
```
Byte 0: Status message ID to request (e.g., 0x05 for PID_DEBUG)
```
**Response:** Requested status message sent immediately

### 0x091: PING (existing)
**Direction:** Pi → ESP32  
**DLC:** 4  
**Payload:**
```
Bytes 0-3: Timestamp (uint32, microseconds, little-endian)
```
**Response:** PONG with same timestamp

---

## Status Message Definitions

### 0x100: HEARTBEAT
**Direction:** ESP32 → Pi  
**DLC:** 1  
**Rate:** 10 Hz continuous  
**Payload:**
```
Byte 0: System state
  0x00 = INIT
  0x01 = IDLE
  0x02 = ENABLED
  0x03 = RUNNING
  0x04 = FAULT
  0x05 = ESTOP
  0x06 = HOMING
  0x07 = TRAJECTORY
```
**Purpose:** Watchdog monitoring, basic health check

### 0x101: MOTOR_STATUS
**Direction:** ESP32 → Pi  
**DLC:** 8  
**Rate:** 10 Hz  
**Payload:**
```
Bytes 0-1: Status flags (uint16, bit field)
  Bit 0: Motor 1 enabled
  Bit 1: Motor 2 enabled
  Bit 2: Motor 1 fault
  Bit 3: Motor 2 fault
  Bit 4: At target (position mode)
  Bit 5: Trajectory active
  Bit 6: Brake 1 engaged
  Bit 7: Brake 2 engaged
  Bit 8: Home position set
  Bit 9: Limit switch 1 active
  Bit 10: Limit switch 2 active
  Bits 11-15: Reserved
Bytes 2-3: Motor 1 error counter (uint16)
Bytes 4-5: Motor 2 error counter (uint16)
Bytes 6-7: Uptime seconds (uint16)
```

### 0x102: POSITION_REPORT
**Direction:** ESP32 → Pi  
**DLC:** 8  
**Rate:** 50 Hz  
**Payload:**
```
Bytes 0-3: Motor 1 position (int32, encoder counts)
Bytes 4-7: Motor 2 position (int32, encoder counts)
```
**Note:** High-rate message for real-time feedback

### 0x103: VELOCITY_REPORT
**Direction:** ESP32 → Pi  
**DLC:** 8  
**Rate:** 50 Hz  
**Payload:**
```
Bytes 0-3: Motor 1 velocity (float32, RPM)
Bytes 4-7: Motor 2 velocity (float32, RPM)
```
**Calculation:** Filtered derivative of position over 5ms window

### 0x104: FAULT_REPORT
**Direction:** ESP32 → Pi  
**DLC:** 4  
**Rate:** On change  
**Payload:**
```
Bytes 0-3: Fault flags (uint32, bit field)
  Bit 0: Motor 1 overcurrent
  Bit 1: Motor 2 overcurrent
  Bit 2: Motor 1 over-temperature
  Bit 3: Motor 2 over-temperature
  Bit 4: Encoder 1 error
  Bit 5: Encoder 2 error
  Bit 6: CAN watchdog timeout
  Bit 7: Position limit exceeded
  Bit 8: Velocity limit exceeded
  Bit 9: Acceleration limit exceeded
  Bit 10: Control loop overrun
  Bit 11: Memory error
  Bit 12: Configuration error
  Bits 13-31: Reserved
```

### 0x105: PID_DEBUG
**Direction:** ESP32 → Pi  
**DLC:** 12 (requires 2 frames)  
**Rate:** 1 Hz or on request  
**Payload (Frame 1):**
```
Byte 0: Motor select (0=Motor1, 1=Motor2)
Bytes 1-4: P term (float32)
Bytes 5-7: (I term bytes 0-2)
```
**Payload (Frame 2):**
```
Byte 0: (I term byte 3)
Bytes 1-4: D term (float32)
Bytes 5-7: Output (float32, bytes 0-2)
```

### 0x106: CURRENT_REPORT
**Direction:** ESP32 → Pi  
**DLC:** 8  
**Rate:** 10 Hz  
**Payload:**
```
Bytes 0-3: Motor 1 current (float32, Amps)
Bytes 4-7: Motor 2 current (float32, Amps)
```

### 0x107: TEMPERATURE_REPORT
**Direction:** ESP32 → Pi  
**DLC:** 4  
**Rate:** 1 Hz  
**Payload:**
```
Bytes 0-1: Driver 1 temperature (int16, °C * 10)
Bytes 2-3: Driver 2 temperature (int16, °C * 10)
```

### 0x108: BUFFER_STATUS
**Direction:** ESP32 → Pi  
**DLC:** 4  
**Rate:** 10 Hz (trajectory mode only)  
**Payload:**
```
Byte 0: Buffer fill level (0-50 points)
Byte 1: Buffer underrun count
Bytes 2-3: Next point timestamp (uint16, ms)
```

### 0x109: LIMIT_SWITCH_STATUS
**Direction:** ESP32 → Pi  
**DLC:** 1  
**Rate:** On change  
**Payload:**
```
Byte 0: Limit switch states
  Bit 0: Limit 1 forward
  Bit 1: Limit 1 reverse
  Bit 2: Limit 2 forward
  Bit 3: Limit 2 reverse
  Bit 4: Home switch 1
  Bit 5: Home switch 2
  Bits 6-7: Reserved
```

### 0x10A: ERROR_LOG
**Direction:** ESP32 → Pi  
**DLC:** 8  
**Rate:** On event  
**Payload:**
```
Bytes 0-3: Timestamp (uint32, ms since boot)
Byte 4: Error code
Byte 5: Error source (0=Motor1, 1=Motor2, 2=System)
Bytes 6-7: Error data (context-dependent)
```

### 0x10B: PERFORMANCE_METRICS
**Direction:** ESP32 → Pi  
**DLC:** 8  
**Rate:** 1 Hz  
**Payload:**
```
Bytes 0-1: Average loop time (uint16, microseconds)
Bytes 2-3: Max loop time (uint16, microseconds)
Byte 4: Loop overrun count
Byte 5: CPU load percentage (0-100)
Bytes 6-7: Free heap memory (uint16, KB)
```

### 0x111: PONG (existing)
**Direction:** ESP32 → Pi  
**DLC:** 4  
**Payload:**
```
Bytes 0-3: Sequence counter (uint32, little-endian)
```

---

## Protocol Invariants (Extensions to Existing)

### Timing Requirements
1. **Command acknowledgment:** < 10ms
2. **Emergency stop response:** < 1ms
3. **Heartbeat period:** 100ms ± 5ms
4. **Position report jitter:** < 2ms
5. **CAN watchdog timeout:** 200ms

### Ordering Requirements
6. **MOTOR_ENABLE before SET_VELOCITY**
7. **SET_HOME_POSITION before GO_TO_HOME**
8. **TRAJECTORY_POINT(s) before TRAJECTORY_START**
9. **SET_MODE before mode-specific commands**

### Safety Requirements
10. **Motors disabled on any fault**
11. **ESTOP clears all motion**
12. **Current limit never exceeded**
13. **Watchdog timeout triggers safe state**
14. **Invalid commands NAKed, not executed**

### Data Integrity
15. **Little-endian byte order (all multi-byte values)**
16. **Checksums on multi-frame messages** (TBD)
17. **Sequence numbers prevent duplication**

---

## Error Handling

### NAK Responses
When ESP32 cannot execute a command:
```
Response: FAULT_REPORT with specific bit set
Example: SET_VELOCITY in IDLE state
  → FAULT_REPORT with configuration error bit
```

### Timeout Handling
**Pi side:**
- If no HEARTBEAT for 200ms → log warning
- If no HEARTBEAT for 500ms → trigger fault recovery
- If no PONG for 100ms → retry PING

**ESP32 side:**
- If no commands for 200ms → log warning  
- If no commands for 1000ms → disable motors (safety)
- If ESTOP, no auto-recovery

---

## Message Priority (CAN Arbitration)

**Highest Priority (Lowest ID):**
1. EMERGENCY_STOP (0x085)
2. MOTOR_ENABLE (0x081)
3. HEARTBEAT (0x100)

**High Priority:**
4. SET_VELOCITY (0x082)
5. POSITION_REPORT (0x102)
6. VELOCITY_REPORT (0x103)

**Medium Priority:**
7. FAULT_REPORT (0x104)
8. TRAJECTORY_POINT (0x08B)
9. MOTOR_STATUS (0x101)

**Low Priority:**
10. Configuration messages (0x084, 0x087-0x08A)
11. Debug messages (0x105, 0x10B)
12. PING/PONG (0x091, 0x111)

---

## Bus Load Analysis

### Worst Case (All messages at max rate):
```
Heartbeat:     10 Hz × 64 bits = 640 bps
Motor Status:  10 Hz × 128 bits = 1280 bps
Position:      50 Hz × 128 bits = 6400 bps
Velocity:      50 Hz × 128 bits = 6400 bps
Current:       10 Hz × 128 bits = 1280 bps
Commands:      ~100 Hz × 128 bits = 12800 bps
--------------------------------
Total: ~29 kbps (5.8% of 500 kbps bus)
```

**Margin:** 94% available for bursts, debug, future expansion

---

## Testing Requirements

### Protocol Validator
- [ ] All message formats parsable
- [ ] Invalid messages rejected
- [ ] Sequence numbers increment correctly
- [ ] Timeouts trigger appropriately

### Stress Tests
- [ ] Maximum message rate sustained
- [ ] No message loss under load
- [ ] Priority arbitration correct
- [ ] Buffer overflow handling

### Safety Tests
- [ ] ESTOP response time < 1ms
- [ ] Watchdog triggers on timeout
- [ ] Fault conditions handled correctly
- [ ] Recovery procedures work

---

## Future Extensions (V3.00.00+)

### Potential Additions:
- Multi-axis coordination (>2 motors)
- Torque control mode
- Force feedback
- Joystick input forwarding
- Configuration download/upload
- Firmware update via CAN
- Network time synchronization (PTP-like)

---

**Status:** Complete specification, ready for implementation  
**Next:** Implement protocol parser/encoder on both sides  
**Dependencies:** None (extends existing PING/PONG)
