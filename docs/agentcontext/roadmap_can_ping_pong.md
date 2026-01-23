# CAN PING### Hardware State
- ✅ **ESP32:** ESP32-WROOM-32D on UPeasy devkit
- ✅ **ESP32 CAN Transceiver:** SN65HVD230 (3.3V logic)
  - **GPIO5 → SN65HVD230 TX** (safe default, no boot conflicts)
  - **GPIO4 → SN65HVD230 RX** (safe default, no boot conflicts)
  - SN65HVD230 RS pin → GND (high-speed mode, max bitrate)
  - SN65HVD230 powered by 3.3V
- ✅ **Raspberry Pi:** Raspberry Pi 3B+
- ✅ **Pi CAN Transceiver:** MCP2515 CAN Bus Module (SPI0 interface)
  - **Failure Modes:**
- Module load fails → check `/boot/config.txt` for MCP2515 overlay
- Interface won't come up → check SPI wiring (MOSI/MISO/SCLK/CS), verify oscillator frequency
- Interrupt not working → verify GPIO25 (Pin 22) connected to MCP2515 INT
- Bitrate error → verify MCP2515 crystal frequency (8MHz or 16MHz) matches overlay config
- Termination issues → measure resistance between CAN_H and CAN_L (should be ~60Ω with both terminators)→ GPIO10 (Pin 19)
  - MISO → GPIO9 (Pin 21)
  - SCLK → GPIO11 (Pin 23)
  - CS → GPIO8 (Pin 24)
  - INT → GPIO25 (Pin 22)
  - VCC → 3.3V (Pin 1)
  - GND → GND (Pin 39)
- ✅ CAN_H and CAN_L connected between SN65HVD230 and MCP2515
- ✅ **Termination resistors:** 120Ω at BOTH ends required (measures ~60Ω total)
- ✅ Twisted pair cable recommended, max length 5m for 500 kbps testing
- ✅ Power supplies stable and **shared ground mandatory**n Roadmap
**Date:** 2026-01-22  
**Goal:** Establish minimal, robust CAN communication between Raspberry Pi and ESP32  
**Scope:** PING/PONG only - no motor control, no encoders, no Wi-Fi  
**Status:** Planning Complete - Ready for Implementation  

---

## 0. Prerequisites & Assumptions

### Hardware State
- ✅ **ESP32:** ESP32-WROOM-32D on UPeasy devkit
- ✅ **ESP32 CAN Transceiver:** SN65HVD230 (3.3V logic)
  - GPIO21 (Pin 33) → SN65HVD230 TX
  - GPIO22 (Pin 36) → SN65HVD230 RX
  - SN65HVD230 powered by 3.3V
- ✅ **Raspberry Pi:** Raspberry Pi 3B+
- ✅ **Pi CAN Transceiver:** MCP2515 CAN Bus Module (SPI interface)
  - INT → GPIO8 (Pin 24)
  - SCK → GPIO10 (Pin 23) [Note: Typo in user info, should be Pin 23 not 19]
  - SI (MOSI) → GPIO9 (Pin 21)
  - SO (MISO) → GPIO11 (Pin 23) 
  - CS → GPIO7 (Pin 26)
  - VCC → 3.3V (Pin 1)
  - GND → GND (Pin 39)
- ✅ CAN_H and CAN_L connected between SN65HVD230 and MCP2515
- ✅ 120Ω termination resistors required at BOTH ends (ESP32 and Pi)
- ✅ Power supplies stable and shared ground established

### Protocol State
- ✅ CAN ID schema defined (11-bit: Class[10:8] | Node[7:4] | Command[3:0])
- ✅ Bitrate: 500 kbps
- ✅ PING command ID exists: 0x001 (CAN ID 0x091)
- ✅ PONG status ID allocated: 0x001 (CAN ID 0x111)
- ✅ Message classes defined (COMMAND=001, STATUS=010, etc.)
- ✅ Complete ID reference table in `protocol/can_id_map.md` (binary, hex, decimal)
- ✅ CAN invariants documented in `protocol/can_invariants.md`

### Architecture State
- ✅ ESP Core 0 reserved for control loop (untouchable)
- ✅ ESP Core 1 available for CAN RX/TX
- ✅ Pi is non-real-time (Python-based)
- ✅ roles.md and motion_contract.md established

---

## 1. Required Planning Documents

Before ANY code is written, create these documents in the appropriate `thoughtprocesses/` folders:

### 1.1 ESP-Side Planning
**Path:** `esp/thoughtprocesses/2026-01-22_can-bringup-esp.md`

**Required Contents:**
- User request verbatim
- **Hardware:** ESP32-WROOM-32D on UPeasy devkit
- **CAN Transceiver:** SN65HVD230 (3.3V logic, RS=GND for high-speed mode)
- **Pin assignments:**
  - **CAN TX: GPIO5** (safe default, overrides previous GPIO21)
  - **CAN RX: GPIO4** (safe default, overrides previous GPIO22)
  - Rationale: GPIO4/5 are safe (no boot conflicts, not SPI flash, not UART0)
  - GPIO4/5 pins: Pin 26 (GPIO4), Pin 29 (GPIO5) on ESP32-WROOM-32D
- TWAI driver configuration (bitrate, timing parameters, queue depths)
- Core affinity strategy (which FreeRTOS task runs where)
- Message parsing approach (how to extract Class/Node/Command from 11-bit ID)
- PING reception behavior (what ESP does when it receives PING)
- PONG transmission behavior (payload format, timing)
- Error handling (bus-off, arbitration lost, RX overflow)
- Initialization sequence (order of operations in setup)
- Testing strategy (how to verify each step works)
- Risks (what could fail and how to detect it)

### 1.2 Pi-Side Planning
**Path:** `pi/thoughtprocesses/2026-01-22_can-bringup-pi.md`

**Required Contents:**
- User request verbatim
- **Hardware:** Raspberry Pi 3B+
- **CAN Transceiver:** MCP2515 SPI module
- **SPI0 Wiring:**
  - MOSI → GPIO10 (Pin 19)
  - MISO → GPIO9 (Pin 21)
  - SCLK → GPIO11 (Pin 23)
  - CS → GPIO8 (Pin 24)
  - INT → GPIO25 (Pin 22)
  - VCC → 3.3V, GND → GND
- SocketCAN interface name: `can0`
- Python CAN library choice (python-can recommended)
- CAN interface bring-up commands (ip link set, bitrate config)
- PING transmission behavior (how often, payload format)
- PONG reception behavior (timeout handling, logging)
- Error handling (interface down, no ACK, timeout)
- Initialization sequence (setup order)
- Testing strategy (manual verification steps)
- Mock/simulation mode support (dry-run without hardware)
- Risks and failure modes

### 1.3 Protocol Planning
**Path:** `protocol/thoughtprocesses/2026-01-22_ping-pong-spec.md`

**Required Contents:**
- User request verbatim
- PING message full specification:
  - CAN ID breakdown (Class=001, Node=0001, Command=0001)
  - Binary: 001 0001 0001
  - Hex: 0x091
  - Decimal: 145 (for logging reference)
  - Payload format: 0 bytes (DLC=0)
  - Expected response (PONG within 100ms)
- PONG message full specification:
  - CAN ID breakdown (Class=010, Node=0001, Status=0001)
  - Binary: 010 0001 0001
  - Hex: 0x111
  - Decimal: 273 (for logging reference)
  - Payload format: 4 bytes uint32 sequence counter (little-endian)
  - Transmission timing (immediate, non-blocking)
- PONG sequence counter behavior:
  - Starts at 0 on ESP boot
  - Increments on each PONG sent
  - Wraps at UINT32_MAX
  - Allows duplicate detection and reboot detection
- Node ID validation:
  - ESP only responds to PING with Node=0x1 (ESP32-1)
  - Pi must validate PONG source Node=0x1
  - Multi-PONG detection is an error (log warning)
- Sequence diagrams (Pi→ESP PING, ESP→Pi PONG)
- Failure cases (timeout, corrupted message, bus error, multi-node response)
- Testing validation criteria
- Reference `protocol/can_id_map.md` for canonical ID table
- Reference `protocol/can_invariants.md` for protocol rules

---

## 2. CAN Message Flow Definition

### 2.1 PING Message (Pi → ESP32)

**Direction:** Raspberry Pi sends, ESP32 receives

**CAN ID Construction:**
```
Message Class: 001 (COMMAND)
Node ID:       0001 (ESP32-1)
Command ID:    0001 (PING)

Binary:  001 0001 0001
Hex:     0x091
Decimal: 145

See protocol/can_id_map.md for complete reference table.
```

**Payload:**
- Length: 0 bytes (DLC=0)
- Rationale: PING requires no parameters

**Pi Behavior:**
1. Bring up SocketCAN interface `can0` at 500 kbps
2. Send PING message with ID=0x091, DLC=0
3. Wait for PONG response (timeout: 100ms recommended)
4. Log success/failure
5. Optionally repeat at configurable interval (e.g., 1 Hz for testing)

**ESP Behavior:**
1. Receive message on TWAI peripheral
2. Parse CAN ID to extract Class/Node/Command
3. Validate: Class==COMMAND (0x1) && Node==ESP32-1 (0x1) && Command==PING (0x1)
4. **Node ID Check:** Ignore if Node != 0x1 (not addressed to this ESP)
5. If valid: trigger PONG transmission
6. If invalid: ignore silently (do not respond, do not log spam)

---

### 2.2 PONG Message (ESP32 → Pi)

**Direction:** ESP32 sends, Raspberry Pi receives

**CAN ID Construction:**
```
Message Class: 010 (STATUS)
Node ID:       0001 (ESP32-1)
Status ID:     0001 (PONG)

Binary:  010 0001 0001
Hex:     0x111
Decimal: 273

See protocol/can_id_map.md for complete reference table.
```

**Payload:**
- **Length: 4 bytes (DLC=4) - REQUIRED**
- **Byte 0-3: uint32 sequence counter (little-endian)**
- **Counter starts at 0 on ESP boot**
- **Counter increments on each PONG transmission**
- **Counter wraps at UINT32_MAX (0xFFFFFFFF)**

**Rationale for 4-byte payload:**
- Detects duplicate frames (same counter value)
- Detects ESP reboot (counter reset to 0)
- Enables round-trip statistics analysis
- Costs minimal bandwidth (4 bytes @ 1 Hz = 32 bps)
- Future-proofs diagnostics without protocol change

**ESP Behavior:**
1. On PING reception, construct PONG message
2. Set CAN ID = 0x111
3. Set DLC = 4
4. Set payload bytes 0-3 = current sequence counter (little-endian)
5. Increment sequence counter (post-increment, wraps automatically)
6. Transmit via TWAI with timeout (10ms, non-blocking)
7. If TX queue full: drop silently, do not block (log warning optional)

**Pi Behavior:**
1. Listen for messages with ID=0x111
2. Validate: DLC == 4 (if not, log error and ignore message)
3. Extract Node ID from CAN ID: `(msg_id >> 4) & 0x0F` == 0x1
4. If Node ID != 0x1: log error "PONG from wrong node" (should never happen)
5. Parse sequence counter from payload bytes 0-3 (little-endian)
6. Check for duplicate counter (same as previous PONG)
7. Check for counter reset (new < previous, likely ESP reboot)
8. Log reception timestamp and sequence number
9. Calculate round-trip time (PING send → PONG receive)
10. Display or store for diagnostics

**Single-Node Assertion:**
- Only one ESP should respond to each PING
- If Pi receives multiple PONGs with different Node IDs → log ERROR
- If Pi receives multiple PONGs with same Node ID → check for duplicate (cable reflection/loop)
- This prevents phantom success when scaling to multi-ESP later

---

## 3. ESP32 Bring-Up Phases

### Phase 3.1: TWAI Driver Initialization
**Exit Criteria:** TWAI driver initializes without error, no panics

**Tasks:**
- Include ESP-IDF TWAI driver headers: `#include "driver/twai.h"`
- **Define GPIO pins for CAN TX/RX:**
  ```c
  #define CAN_TX_GPIO 5   // GPIO5, Pin 29 on ESP32-WROOM-32D
  #define CAN_RX_GPIO 4   // GPIO4, Pin 26 on ESP32-WROOM-32D
  ```
- Configure `twai_general_config_t`:
  - Mode: `TWAI_MODE_NORMAL`
  - TX GPIO: `GPIO_NUM_5`
  - RX GPIO: `GPIO_NUM_4`
  - Clkout disabled: `TWAI_IO_UNUSED`
  - Bus-off recovery enabled: `TWAI_ALERT_BUS_OFF`
  - Note: SN65HVD230 RS pin must be grounded for high-speed mode (120 kbps - 1 Mbps)
- Configure `twai_timing_config_t`:
  - Use `TWAI_TIMING_CONFIG_500KBITS()` macro
- Configure `twai_filter_config_t`:
  - **Initial testing:** Accept all messages (single word filter, mask 0x000)
  - **Production configuration (document for future):**
    - Filter for Class=COMMAND (001) OR Class=ERROR (100)
    - Node=ESP32-1 (0001)
    - Acceptance mask: `0x1F0` (bits 8,7,6,5,4 matter)
    - Acceptance code: `0x090` (matches 0x09X = COMMAND to ESP32-1)
    - Alternative: Dual filter mode for COMMAND and ERROR classes
  - Note: Keep filter disabled during bring-up for easier debugging
- Call `twai_driver_install()`
- Call `twai_start()`
- Verify driver status with `twai_get_status_info()`

**Testing:**
- ESP boots without crash
- Serial log shows "TWAI driver started"
- No error flags in status

**Failure Modes:**
- GPIO conflict with other peripherals → check pinout
- Timing config mismatch → verify bitrate calculation
- Bus-off at startup → check wiring, termination

---

### Phase 3.2: CAN RX Task on Core 1
**Exit Criteria:** RX task runs, can receive messages, does not block Core 0

**Tasks:**
- Create FreeRTOS task: `can_rx_task()`
- Pin task to Core 1 using `xTaskCreatePinnedToCore()`
- Set priority below control loop (e.g., priority 5 if control is 10)
- Inside task loop:
  - Call `twai_receive()` with timeout (e.g., 50ms)
  - If message received: parse ID and dispatch
  - If timeout: continue (do not log spam)
- Implement `parse_can_id()` helper:
  ```c
  uint8_t class = (id >> 8) & 0x07;
  uint8_t node  = (id >> 4) & 0x0F;
  uint8_t cmd   = (id >> 0) & 0x0F;
  ```
- Dispatch to handler based on Class/Command

**Testing:**
- Send test CAN frame from Pi (any ID)
- ESP logs "CAN RX: received message ID=0xXXX"
- Control loop continues unaffected (check via separate LED or telemetry)

**Failure Modes:**
- Task blocks Core 0 → verify core affinity in task creation
- RX queue overflow → increase queue depth or reduce RX load
- Missed messages → check filter settings

---

### Phase 3.3: PING Handler Implementation
**Exit Criteria:** ESP receives PING, logs it, does NOT yet send PONG

**Tasks:**
- In CAN RX task dispatch logic, add case for PING:
  ```c
  if (class == CLASS_COMMAND && node == NODE_ESP32_1 && cmd == CMD_PING) {
      handle_ping(message);
  } else if (node != NODE_ESP32_1) {
      // Silently ignore - not addressed to this node
      // Do not log (prevents spam in multi-node networks)
  }
  ```
- Implement `handle_ping()`:
  - Log "PING received from Pi" with timestamp
  - Increment internal counter (for debugging)
  - DO NOT transmit yet (testing RX only)
  - Validate this is only ESP node responding (no multi-node yet)

**Testing:**
- Pi sends PING (ID=0x091)
- ESP logs "PING received"
- ESP does not respond (expected at this phase)
- Control loop unaffected

**Failure Modes:**
- PING not detected → verify ID parsing logic
- ESP crashes → check stack size, avoid dynamic allocation

---

### Phase 3.4: CAN TX Implementation
**Exit Criteria:** ESP can transmit PONG in response to PING, with sequence counter

**Tasks:**
- Add global variable: `static uint32_t pong_sequence = 0;`
- Implement `can_tx_pong()` function:
  - Construct `twai_message_t`:
    - `identifier = 0x111` (PONG ID from can_id_map.md)
    - `data_length_code = 4` (uint32 payload)
    - `flags = TWAI_MSG_FLAG_NONE` (standard frame)
    - `data[0-3] = pong_sequence` (little-endian: LSB in data[0])
  - Call `twai_transmit()` with short timeout (10ms)
  - If timeout: log warning "TX queue full", do not block
  - Increment `pong_sequence++` after successful transmit (wraps automatically)
- Call `can_tx_pong()` from `handle_ping()`
- Verify little-endian encoding:
  - Example: sequence=0x12345678 → data[0]=0x78, data[1]=0x56, data[2]=0x34, data[3]=0x12

**Testing:**
- Pi sends PING
- ESP logs "PING received"
- ESP logs "PONG transmitted, seq=0"
- Pi receives PONG (ID=0x111, DLC=4)
- Pi decodes sequence counter correctly
- Send multiple PINGs: sequence increments (0, 1, 2, ...)
- Round-trip < 50ms (measure on Pi side)

**Failure Modes:**
- TX fails with error → check bus wiring, ACK presence
- PONG not received on Pi → verify Pi RX filter, check ID
- Sequence counter wrong endianness → verify byte order
- Counter doesn't increment → check post-increment logic

---

### Phase 3.5: Error Handling & Robustness
**Exit Criteria:** ESP gracefully handles bus errors, does not crash or block

**Tasks:**
- Monitor `twai_get_status_info()` periodically:
  - If `state == TWAI_STATE_BUS_OFF`: attempt recovery with `twai_initiate_recovery()`
  - Log error counters (TX/RX error counts)
- Add timeout for TX operations (already in 3.4)
- Add bounds checking for message parsing
- Ensure no dynamic allocation in RX/TX paths
- Ensure no blocking calls in control loop (verify isolation)

**Testing:**
- Disconnect CAN cable during operation → ESP logs error, attempts recovery
- Reconnect cable → ESP resumes normal PING/PONG
- Send malformed CAN frame → ESP ignores gracefully
- Send high-rate PING (10 Hz) → ESP responds without dropping

**Failure Modes:**
- ESP stuck in bus-off → check recovery logic
- Watchdog timeout → verify no blocking calls

---

## 4. Raspberry Pi Bring-Up Phases

### Phase 4.1: SocketCAN Interface Configuration
**Exit Criteria:** `can0` interface up, bitrate set, no errors

**Tasks:**
- **Verify SPI is enabled in `/boot/config.txt`:**
  ```bash
  # Should contain: dtparam=spi=on
  # Should contain MCP2515 overlay:
  # dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25,spimaxfrequency=1000000
  # Note: Adjust oscillator to match your MCP2515 crystal (8MHz or 16MHz)
  # interrupt=25 corresponds to GPIO25 (Pin 22)
  # spimaxfrequency=1000000 is conservative; can tune 1-10 MHz
  ```
- Load kernel modules:
  ```bash
  sudo modprobe can
  sudo modprobe can_raw
  sudo modprobe mcp251x  # MCP2515 driver
  ```
- Configure interface:
  ```bash
  sudo ip link set can0 type can bitrate 500000
  sudo ip link set can0 up
  ```
- Verify:
  ```bash
  ip -details link show can0
  # Should show UP, bitrate 500000
  ```
- Check for errors:
  ```bash
  dmesg | grep -i can
  # Look for initialization messages, no errors
  ```

**Testing:**
- Interface appears in `ip link`
- State is UP
- No error frames in `dmesg`

**Failure Modes:**
- Module load fails → check `/boot/config.txt` for MCP2515 overlay
- Interface won't come up → check SPI wiring (MOSI/MISO/SCK/CS), verify oscillator frequency
- Interrupt not working → verify GPIO8 (Pin 24) connected to MCP2515 INT
- Bitrate error → verify MCP2515 crystal frequency (8MHz or 16MHz) matches overlay config

---

### Phase 4.2: Python CAN Library Setup
**Exit Criteria:** Python can send/receive raw CAN frames

**Tasks:**
- Install `python-can` library:
  ```bash
  pip3 install python-can
  ```
- Create test script `pi/can_test.py`:
  ```python
  import can
  
  bus = can.interface.Bus(channel='can0', bustype='socketcan')
  
  # Send test message
  msg = can.Message(arbitration_id=0x123, data=[1, 2, 3], is_extended_id=False)
  bus.send(msg)
  print("Sent test message")
  
  # Receive test message (timeout 5s)
  msg = bus.recv(timeout=5.0)
  if msg:
      print(f"Received: ID={hex(msg.arbitration_id)}, Data={msg.data.hex()}")
  else:
      print("No message received")
  ```
- Run test with ESP disconnected (expect no RX)
- Run test with ESP connected (expect loopback if ESP echoes)

**Testing:**
- Script runs without import errors
- CAN frames sent (verify with `candump can0` in separate terminal)
- No Python exceptions

**Failure Modes:**
- Import error → reinstall python-can, check virtualenv
- Permission denied → add user to `netdev` group, reboot
- No frames sent → check interface state

---

### Phase 4.3: PING Transmission Implementation
**Exit Criteria:** Pi sends PING at 1 Hz, logs each send

**Tasks:**
- Create `pi/can/can_tx_rx.py` module:
  - Initialize SocketCAN bus in constructor
  - Implement `send_ping(node_id)`:
    - Construct CAN ID: `(0x1 << 8) | (node_id << 4) | 0x1`
    - Create `can.Message` with DLC=0
    - Call `bus.send()`
    - Log timestamp and ID
    - Return success/failure
- Create `pi/test_ping.py`:
  - Loop at 1 Hz
  - Call `send_ping(node_id=0x1)` for ESP32-1
  - Log result
  - Catch exceptions (interface down, etc.)

**Testing:**
- Run `python3 test_ping.py`
- See "Sent PING to ESP32-1 at <timestamp>"
- Verify with `candump can0` → see ID=0x091 every 1 second
- ESP logs "PING received" (if Phase 3.3 complete)

**Failure Modes:**
- No ACK → ESP not powered or not running CAN RX
- Bus error → check wiring, termination

---

### Phase 4.4: PONG Reception Implementation
**Exit Criteria:** Pi receives PONG, calculates round-trip time, logs

**Tasks:**
- In `can_tx_rx.py`, implement `wait_for_pong(timeout_ms)`:
  - Call `bus.recv(timeout=timeout_ms/1000.0)`
  - Check `msg.arbitration_id == 0x111` (PONG ID)
  - Validate DLC == 4 (if not, log error and return None)
  - Parse node ID from message: `(msg.arbitration_id >> 4) & 0x0F`
  - Verify node ID == 0x1 (ESP32-1)
  - **Single-node assertion:** Track if multiple PONGs received
  - Parse sequence counter from payload (little-endian):
    ```python
    seq = int.from_bytes(msg.data[0:4], byteorder='little', signed=False)
    ```
  - Check for duplicate (seq == previous_seq) → log warning
  - Check for reboot detection (seq < previous_seq and delta > 100) → log "ESP reboot detected"
  - Update previous_seq
  - Return message and sequence number
- In `test_ping.py`:
  - After sending PING, call `wait_for_pong(timeout_ms=100)`
  - If received: calculate `time_rx - time_tx`, log RTT and sequence
  - If timeout: log "No PONG received"
  - If multiple PONGs: log ERROR "Multiple nodes responding"

**Testing:**
- Run `python3 test_ping.py`
- See "Sent PING" followed by "Received PONG, seq=0, RTT=X ms"
- Next PING: "Received PONG, seq=1, RTT=Y ms"
- RTT < 50ms typical, < 100ms acceptable
- ESP logs match (PING RX, PONG TX with seq)
- Restart ESP mid-test → Pi detects sequence reset

**Failure Modes:**
- Timeout → ESP not responding, check Phase 3.4
- Wrong ID received → verify PONG ID allocation
- Wrong DLC → verify ESP payload length
- Sequence doesn't increment → check ESP counter logic
- Endianness error → verify little-endian on both sides

---

### Phase 4.5: Robust Error Handling
**Exit Criteria:** Pi handles interface errors, ESP restarts, CAN bus errors gracefully

**Tasks:**
- Wrap all CAN operations in try/except:
  - Catch `can.CanError` → log, retry or exit
  - Catch `OSError` → check interface state, attempt recovery
- Implement interface health check:
  - Periodically call `ip link show can0` via subprocess
  - If DOWN: attempt `ip link set can0 up`
- Add statistics tracking:
  - PING count sent
  - PONG count received
  - Timeout count
  - Success rate percentage
  - Sequence counter statistics (gaps, duplicates, resets)
  - Multi-node response count (should be 0)
- Add graceful shutdown:
  - `bus.shutdown()` on KeyboardInterrupt

**Testing:**
- Run test, press Ctrl+C → clean shutdown, no exceptions
- Restart ESP during operation → Pi logs timeout, recovers on next PING
- Bring `can0` down manually → Pi detects, logs error
- Bring `can0` up → Pi recovers

**Failure Modes:**
- Pi crashes on interface error → improve exception handling
- Stats incorrect → verify counter logic

---

## 5. Validation & Test Steps

### 5.1 Per-Phase Testing
Each phase above includes specific tests. Do not proceed to next phase until current phase exit criteria are met.

### 5.2 Integration Test Sequence

**Test 1: Single PING/PONG**
- Pi sends one PING
- ESP responds with one PONG
- Pi logs success
- **Pass Criteria:** RTT < 100ms, no errors

**Test 2: Continuous PING/PONG (1 Hz)**
- Pi sends PING every 1 second for 1 minute
- ESP responds to each PING
- **Pass Criteria:** >95% success rate, no crashes

**Test 3: High-Rate PING/PONG (10 Hz)**
- Pi sends PING every 100ms for 10 seconds
- ESP responds to each PING
- **Pass Criteria:** >90% success rate, control loop unaffected

**Test 4: ESP Restart During Operation**
- Start continuous PING/PONG
- Restart ESP (power cycle or software reset)
- **Pass Criteria:** Pi logs timeouts during restart, recovers automatically when ESP comes back

**Test 5: CAN Interface Restart**
- Start continuous PING/PONG
- Bring `can0` down on Pi: `sudo ip link set can0 down`
- Bring `can0` back up: `sudo ip link set can0 up`
- **Pass Criteria:** Pi logs error, recovers automatically

**Test 6: Bus Error Injection**
- Disconnect CAN cable during operation
- **Pass Criteria:** ESP logs bus-off, attempts recovery; Pi logs timeouts
- Reconnect cable
- **Pass Criteria:** Both nodes recover automatically

**Test 7: Malformed Message Handling**
- Use `cansend` to manually send invalid CAN IDs
- Send PING with wrong node ID (e.g., Node=0x2)
- Send PONG directly to Pi (should be ignored)
- Send message with wrong DLC
- **Pass Criteria:** ESP ignores gracefully, no crash, only addressed messages processed

**Test 8: Multi-Node Detection**
- Single-node setup: verify only one PONG per PING
- Pi logs error if multiple PONGs detected
- **Pass Criteria:** Pi correctly identifies multi-node responses (test setup for future)

**Test 9: Sequence Counter Validation**
- Verify counter starts at 0 on ESP boot
- Verify counter increments on each PONG
- Restart ESP mid-test: verify Pi detects reset
- Send rapid PINGs: verify no sequence gaps (or log gaps)
- **Pass Criteria:** Counter behavior matches specification

**Test 10: Long-Duration Stability**
- Run continuous PING/PONG (1 Hz) for 24 hours
- Monitor sequence counter: should reach ~86,400
- **Pass Criteria:** >99% success rate, no memory leaks, no crashes, counter wraps correctly if tested long enough

---

## 6. Failure Modes & Detection

### 6.1 Hardware Failures

| Failure | Symptom | Detection Method | Recovery |
|---------|---------|-----------------|----------|
| CAN cable disconnected | No ACK, bus-off | TWAI status, TX error count | Reconnect cable, auto-recovery |
| Missing termination | Bit errors, unstable | High error counters | Add 120Ω resistors |
| Wrong bitrate | No communication | No ACK, bus-off | Verify both ends 500 kbps |
| Transceiver power loss | Interface down | `ip link` state, no RX | Check power supply |
| GPIO misconfiguration | Driver init fails | ESP panic on startup | Fix GPIO assignment |

---

### 6.2 Software Failures

| Failure | Symptom | Detection Method | Recovery |
|---------|---------|-----------------|----------|
| ESP stuck in loop | Watchdog timeout | WDT panic, reboot | Fix blocking code |
| Core 0 blocked by CAN | Control loop stalls | LED blink stops | Verify task affinity |
| Pi interface not up | `OSError` on send | Exception in Python | Run bring-up script |
| RX queue overflow | Dropped messages | TWAI status flags | Increase queue size |
| TX queue full | PONG not sent | TX timeout log | Reduce TX rate or increase queue |
| Memory leak | Crash after hours | Heap monitoring | Fix dynamic allocation |
| ID parsing bug | PING not detected | ESP logs no PING | Test with known IDs |

---

### 6.3 Protocol Failures

| Failure | Symptom | Detection Method | Recovery |
|---------|---------|-----------------|----------|
| Wrong CAN ID used | No response | Wireshark/candump log | Fix ID construction |
| Wrong node ID | ESP ignores | Check node filtering | Verify node ID scheme |
| Mismatched DLC | Payload error | Length check fails | Fix payload format |
| Endianness error | Corrupted data | Value out of range | Use little-endian consistently |

---

## 7. Exit Criteria Before Motion Commands

Before ANY motion-related commands (SET_TARGET_POSITION, etc.) are implemented, ALL of the following must be true:

### 7.1 Communication Reliability
- [ ] PING/PONG success rate >99% over 24-hour test
- [ ] RTT consistently <50ms (p99 <100ms)
- [ ] No memory leaks detected on either node
- [ ] No crashes or panics in 24-hour test

### 7.2 Error Handling
- [ ] ESP recovers from bus-off automatically
- [ ] Pi recovers from interface restart
- [ ] Both nodes handle cable disconnect/reconnect
- [ ] Invalid messages ignored gracefully

### 7.3 Core Isolation
- [ ] ESP Core 0 control loop runs uninterrupted during CAN activity
- [ ] CAN RX/TX task pinned to Core 1 verified
- [ ] No blocking calls in control loop (audited)

### 7.4 Documentation
- [ ] All planning documents complete (Section 1)
- [ ] Code comments reference planning docs with version tags
- [ ] `version_log.md` updated with PING/PONG implementation
- [ ] Failure recovery procedures documented

### 7.5 Testing
- [ ] All tests in Section 5.2 passed
- [ ] Test results logged and archived
- [ ] Known issues documented with workarounds

### 7.6 Protocol Validation
- [ ] PONG command ID officially allocated in `can_id_map.md` (0x111)
- [ ] CAN ID construction verified with oscilloscope or logic analyzer
- [ ] Payload formats match specification exactly (4-byte counter, little-endian)
- [ ] Sequence counter behavior verified (starts at 0, increments, wraps)
- [ ] Single-node response verified (no multi-PONG per PING)
- [ ] Node ID filtering tested (wrong node ignored)
- [ ] CAN invariants documented and followed (see `protocol/can_invariants.md`)

---

## 8. Future Work (OUT OF SCOPE)

The following are explicitly NOT part of this roadmap and must be planned separately:

- Motor control commands (SET_TARGET_POSITION, etc.)
- Encoder integration and reporting
- PID loop activation
- Safety timeout mechanisms
- Telemetry streaming
- Multi-ESP node support
- CAN message filtering optimization
- Bootloader/OTA updates over CAN

Each of these requires a separate roadmap document following the same planning discipline.

---

## 9. Implementation Sequence Summary

**Recommended Order:**
1. Create all planning documents (Section 1)
2. Verify PONG ID in `protocol/can_id_map.md` (0x111 - already allocated)
3. Review `protocol/can_invariants.md` for binding rules
4. ESP Phase 3.1: TWAI init (document filter config even if disabled)
5. Pi Phase 4.1: SocketCAN bring-up
6. Pi Phase 4.2: Python CAN library test
7. ESP Phase 3.2: RX task on Core 1
8. ESP Phase 3.3: PING handler with node ID filtering (logging only)
9. Pi Phase 4.3: PING transmission
10. Validate: Pi sends, ESP logs (no response yet)
11. ESP Phase 3.4: PONG transmission with 4-byte sequence counter
12. Pi Phase 4.4: PONG reception with sequence validation and single-node assertion
13. Validate: Full PING/PONG cycle works, counter increments
14. ESP Phase 3.5: Error handling
15. Pi Phase 4.5: Error handling with sequence statistics
16. Run all tests (Section 5.2) including sequence counter tests
17. Verify exit criteria (Section 7)
18. Update `version_log.md`

**Do not skip steps. Do not proceed if a phase fails.**

---

## 10. Architecture Compliance Checklist

Before implementation, verify:
- [ ] No code violates `docs/roles.md` (ESP does real-time, Pi does planning)
- [ ] No code violates `protocol/motion_contract.md` (not applicable yet, but aware)
- [ ] No code violates `protocol/can_invariants.md` (review all 15 invariants)
- [ ] ESP Core 0 remains dedicated to control loop
- [ ] All CAN operations are non-blocking on ESP
- [ ] No dynamic memory allocation in ESP hot paths
- [ ] Pi code includes mock/simulation mode for testing without hardware
- [ ] All planning documents created BEFORE code
- [ ] All code changes include version-tagged comments referencing planning docs
- [ ] CAN ID math uses canonical table from `protocol/can_id_map.md` (binary, hex, decimal)
- [ ] Payload encoding verified (little-endian, 4-byte counter)
- [ ] Node ID validation implemented (ESP ignores wrong node, Pi validates source)
- [ ] Single-node assertion in place (Pi detects multi-PONG)

---

## 11. Risk Assessment

**High Risk:**
- ESP control loop blocked by CAN → **MITIGATION:** Core affinity enforcement, no blocking calls
- Bus-off unrecoverable → **MITIGATION:** Automatic recovery logic, monitoring

**Medium Risk:**
- Timing jitter >100ms → **MITIGATION:** Profiling, optimization if needed
- ID collision with future commands → **MITIGATION:** Allocate IDs carefully, reserve ranges

**Low Risk:**
- Python library compatibility → **MITIGATION:** Pin version in requirements.txt
- Hardware failure during test → **MITIGATION:** Visual inspection, spare parts

---

## 12. Success Metrics

At completion, we should observe:
- PING sent from Pi every 1 second
- PONG received within 50ms average
- Success rate >99% over 24 hours
- No ESP panics or Pi crashes
- Clean logs with structured messages
- Round-trip time histogram available for analysis

---

## 13. Human Review Points

Before starting implementation, human must confirm:
1. ~~PONG command ID allocation~~ ✅ **APPROVED:** 0x001 under STATUS class (CAN ID 0x111)
2. ~~PONG payload format~~ ✅ **APPROVED:** 4 bytes, uint32 sequence counter, little-endian
3. ESP GPIO pin assignments for CAN TX/RX (awaiting confirmation)
4. SocketCAN interface name on Pi (can0 vs can1, etc.) (awaiting confirmation)
5. Acceptable RTT threshold (currently 100ms, adjust if needed)

**New Protocol Files Created:**
- `protocol/can_id_map.md` updated with complete ID reference table (binary/hex/decimal)
- `protocol/can_invariants.md` created with 15 binding invariants

**Key Decisions Made:**
- PONG ID: 0x111 (STATUS class, see can_id_map.md table)
- PONG payload: 4 bytes (enables duplicate detection, reboot detection, diagnostics)
- Node ID validation: ESP ignores wrong node, Pi validates source
- Single-node assertion: Pi detects and logs multi-PONG responses
- CAN filters: Documented for future, disabled during bring-up

---

## 13. Human Review Points

**Hardware Configuration - APPROVED ✅**

1. ~~PONG command ID allocation~~ ✅ **APPROVED:** 0x001 under STATUS class (CAN ID 0x111)

2. ~~ESP32 Hardware~~ ✅ **CONFIRMED:**
   - Board: ESP32-WROOM-32D on UPeasy devkit
   - Transceiver: SN65HVD230 (3.3V logic)
   - **CAN TX: GPIO5 (Pin 29)**
   - **CAN RX: GPIO4 (Pin 26)**
   - Note: SN65HVD230 RS pin must be grounded for high-speed mode

3. ~~Raspberry Pi Hardware~~ ✅ **CONFIRMED:**
   - Board: Raspberry Pi 3B+
   - Transceiver: MCP2515 CAN Bus Module (SPI0 interface)
   - SPI Pins: MOSI=GPIO10, MISO=GPIO9, SCLK=GPIO11, CS=GPIO8, INT=GPIO25
   - Interface name: `can0`

4. ~~RTT Threshold~~ ✅ **APPROVED:** <50ms avg, <100ms max

**Action Items Before Implementation:**

5. ⚠️ **WIRING REQUIRED:** Connect SN65HVD230 to ESP32:
   - SN65HVD230 TX → ESP32 **GPIO5 (Pin 29)**
   - SN65HVD230 RX → ESP32 **GPIO4 (Pin 26)**
   - SN65HVD230 3.3V → ESP32 3V3 (Pin 2)
   - SN65HVD230 GND → ESP32 GND (Pin 1, 15, or 38)
   - SN65HVD230 RS → GND (for high-speed mode)

6. ⚠️ **TERMINATION REQUIRED:** Install 120Ω resistors:
   - One resistor between CAN_H and CAN_L at ESP32/SN65HVD230 end
   - One resistor between CAN_H and CAN_L at Pi/MCP2515 end
   - Verify: Total resistance measures ~60Ω between CAN_H and CAN_L
   - (If bus <1m for testing, one resistor may suffice, but two is proper)

7. ⚠️ **Pi CONFIGURATION:** Verify `/boot/config.txt` contains:
   ```
   dtparam=spi=on
   dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25
   ```
   (Adjust oscillator=8000000 to 16000000 if your MCP2515 has 16MHz crystal)
   (interrupt=25 corresponds to GPIO25 on Pin 22)

---

## End of Roadmap

This roadmap is complete and ready for implementation.  
All future code agents must follow this sequence exactly.  
Any deviations require explicit human approval and roadmap update.
