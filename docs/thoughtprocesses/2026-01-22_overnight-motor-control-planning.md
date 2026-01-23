# Overnight Planning Run - CNC Motor Control System

**Date:** 2026-01-22  
**Branch:** overnightRun  
**Duration:** Long-term planning session  
**User Request:** "plan out as much of the planned motor control system for a CNC style wheele movement. Detail each and every step. document it and check yourself."

---

## Mission Statement

Design a complete, production-ready motor control system for CNC-style wheel movement on the ESP32/Raspberry Pi distributed architecture. This system will enable precise, coordinated motion control with real-time feedback, safety features, and high-level trajectory planning.

---

## System Architecture Overview

### Hardware Stack
```
┌─────────────────────────────────────────┐
│         Raspberry Pi 3B+                │
│  (High-Level Controller)                │
│  - Trajectory Planning                  │
│  - G-Code Interpretation               │
│  - UI/Monitoring                        │
│  - CAN Commander                        │
└────────────┬────────────────────────────┘
             │ CAN Bus (500 kbps)
             │ PING/PONG Protocol
┌────────────┴────────────────────────────┐
│         ESP32-WROOM-32D                 │
│  (Real-Time Controller)                 │
│  - Motor PWM Control (2 motors)         │
│  - Encoder Reading (2 encoders)         │
│  - PID Control Loops (2 kHz)            │
│  - Safety Monitoring                    │
│  - CAN Communication                    │
└─────┬──────────┬──────────┬─────────────┘
      │          │          │
   Motor 1    Motor 2   Encoders
   Driver     Driver    (PCNT)
```

### Software Architecture
```
Raspberry Pi Side:
├── Motion Planner (Python)
│   ├── Trajectory Generator
│   ├── Kinematic Solver
│   └── Lookahead Buffer
├── CAN Interface (python-can)
│   ├── Command Sender
│   ├── Status Receiver
│   └── Protocol Handler
└── UI/Monitoring
    ├── Web Dashboard
    ├── CLI Tools
    └── G-Code Parser

ESP32 Side:
├── Core 0: Real-Time Control Loop (2 kHz)
│   ├── Encoder Reading (PCNT)
│   ├── Velocity Calculation
│   ├── PID Controllers (2x)
│   ├── Acceleration Limiting
│   ├── PWM Output
│   └── Safety Checks
├── Core 1: Communication & Non-RT
│   ├── CAN RX/TX (TWAI)
│   ├── Command Queue
│   ├── Status Reporting
│   └── Diagnostics
└── Shared State (Mutex Protected)
    ├── Target Velocities
    ├── Current Positions
    ├── Error Flags
    └── Configuration
```

---

## Phase Breakdown (Detailed)

### Phase 0: Foundation (COMPLETE ✅)
- Hardware wiring verified
- CAN bus operational
- Hello world tested
- TWAI driver initialized

### Phase 1: Basic CAN Communication (IN PROGRESS 🔄)
**Current Status:** Phase 2.2 - Test frame TX
**Blocker:** Pi SD card corruption
**Next:** Complete PING/PONG protocol

### Phase 2: Single Motor Control (Velocity Mode)
**Goal:** Control one motor at specified velocity

#### Phase 2.1: PWM Output
- Configure PWM channels (LEDC)
- Motor driver interface (DIR + PWM)
- Test at various duty cycles
- Verify motor response

#### Phase 2.2: Encoder Reading
- Configure PCNT (Pulse Counter)
- Handle quadrature signals
- Edge detection setup
- Overflow handling
- Position/velocity calculation

#### Phase 2.3: Open-Loop Velocity
- Set target velocity
- Convert velocity → PWM
- No feedback yet
- Ramp up/down limits

#### Phase 2.4: PID Velocity Control
- Implement PID controller
- Tune gains (Kp, Ki, Kd)
- Closed-loop velocity tracking
- Anti-windup protection

#### Phase 2.5: CAN Velocity Commands
- Receive velocity setpoint via CAN
- Update PID target
- Report actual velocity
- Handle enable/disable

### Phase 3: Dual Motor Coordination
**Goal:** Control two motors independently

#### Phase 3.1: Second Motor Setup
- Duplicate PWM/encoder config
- Second PID controller
- Independent control

#### Phase 3.2: Synchronized Control
- Simultaneous velocity commands
- Phase synchronization
- Coordinated acceleration

#### Phase 3.3: Differential Drive
- Left/right wheel velocities
- Convert linear/angular → wheel speeds
- Kinematic transforms

### Phase 4: Position Control
**Goal:** Move to absolute positions

#### Phase 4.1: Position Tracking
- Integrate velocity to position
- Encoder position reading
- Overflow handling (wrapping)
- Position reset/homing

#### Phase 4.2: Position Mode PID
- Position → velocity conversion
- Feed-forward acceleration
- Position hold behavior
- Dead-band handling

#### Phase 4.3: Point-to-Point Moves
- Trapezoidal velocity profiles
- Acceleration limits
- Max velocity limits
- Move completion detection

### Phase 5: Trajectory Execution
**Goal:** Execute smooth, coordinated paths

#### Phase 5.1: Trajectory Buffer
- Queue of setpoints
- Streaming interface
- Buffer management
- Underflow handling

#### Phase 5.2: Interpolation
- Linear interpolation
- Cubic splines
- Jerk limiting
- Smooth transitions

#### Phase 5.3: Real-Time Streaming
- 100Hz setpoint updates
- Lookahead buffering
- Adaptive feed rate
- Path blending

### Phase 6: Safety Systems
**Goal:** Protect hardware and ensure safe operation

#### Phase 6.1: Limit Switches
- GPIO input configuration
- Software limits
- Emergency stop handling
- Safe state transitions

#### Phase 6.2: Current Monitoring
- ADC current sensing
- Overcurrent detection
- Thermal protection
- Fault recovery

#### Phase 6.3: Watchdog
- CAN communication timeout
- Heartbeat monitoring
- Automatic disable on fault
- Manual re-enable required

### Phase 7: Calibration & Tuning
**Goal:** Optimize performance for specific hardware

#### Phase 7.1: Auto-Tuning
- Step response test
- System identification
- Automatic PID gain calculation
- Bode plot analysis

#### Phase 7.2: Kinematic Calibration
- Wheel diameter measurement
- Wheelbase measurement
- Encoder resolution
- Backlash compensation

#### Phase 7.3: Performance Testing
- Velocity accuracy
- Position repeatability
- Following error
- Path accuracy

### Phase 8: High-Level Planning (Pi Side)
**Goal:** Generate motion plans from user input

#### Phase 8.1: Trajectory Generator
- Acceleration-limited profiles
- S-curve acceleration
- Multi-axis coordination
- Time-optimal planning

#### Phase 8.2: G-Code Parser
- Basic G0/G1 (rapid/linear)
- G2/G3 (arcs)
- Feedrate control (F)
- Spindle control (M3/M5)

#### Phase 8.3: Path Optimization
- Lookahead buffer
- Corner blending
- Adaptive feedrate
- Continuous motion

### Phase 9: User Interface
**Goal:** Provide control and monitoring

#### Phase 9.1: CLI Tools
- Jog commands
- Position query
- Velocity control
- Parameter setting

#### Phase 9.2: Web Dashboard
- Real-time position display
- Velocity plots
- Error visualization
- Emergency stop button

#### Phase 9.3: G-Code Interface
- File upload
- Real-time status
- Pause/resume
- Progress tracking

### Phase 10: Testing & Validation
**Goal:** Ensure system reliability

#### Phase 10.1: Unit Tests
- PID controller tests
- Kinematic tests
- Buffer management tests
- Protocol tests

#### Phase 10.2: Integration Tests
- End-to-end motion
- Multi-axis coordination
- Fault injection
- Recovery testing

#### Phase 10.3: Performance Tests
- 24-hour stability run
- Precision measurement
- Thermal testing
- Load testing

---

## Detailed Technical Specifications

### Motor Control Loop (ESP32 Core 0)

```cpp
// Control loop timing
#define CONTROL_FREQ_HZ 2000
#define CONTROL_PERIOD_US (1000000 / CONTROL_FREQ_HZ)

// Motor parameters
#define MOTOR_1_PWM_PIN 12
#define MOTOR_1_DIR_PIN 13
#define MOTOR_1_ENC_A_PIN 16
#define MOTOR_1_ENC_B_PIN 17

#define MOTOR_2_PWM_PIN 14
#define MOTOR_2_DIR_PIN 15
#define MOTOR_2_ENC_A_PIN 18
#define MOTOR_2_ENC_B_PIN 19

// PWM configuration
#define PWM_FREQ_HZ 20000  // 20 kHz PWM
#define PWM_RESOLUTION 10   // 10-bit (0-1023)

// Encoder configuration
#define ENCODER_PPR 600     // Pulses per revolution
#define GEAR_RATIO 1.0      // Direct drive

// PID parameters (to be tuned)
#define KP_VELOCITY 0.5
#define KI_VELOCITY 0.1
#define KD_VELOCITY 0.01
#define MAX_INTEGRAL 1000.0

// Safety limits
#define MAX_VELOCITY_RPM 120
#define MAX_ACCELERATION_RPM_S 1000
#define MAX_CURRENT_MA 5000
#define WATCHDOG_TIMEOUT_MS 100
```

### Control Loop Structure

```cpp
void control_loop_task(void* arg) {
    uint64_t last_time = esp_timer_get_time();
    
    while (true) {
        uint64_t now = esp_timer_get_time();
        uint64_t elapsed = now - last_time;
        
        if (elapsed >= CONTROL_PERIOD_US) {
            last_time += CONTROL_PERIOD_US;
            
            // 1. Read encoders (PCNT hardware)
            int32_t pos1 = read_encoder_position(MOTOR_1);
            int32_t pos2 = read_encoder_position(MOTOR_2);
            
            // 2. Calculate velocities (differentiate position)
            float vel1 = calculate_velocity(pos1, CONTROL_PERIOD_US);
            float vel2 = calculate_velocity(pos2, CONTROL_PERIOD_US);
            
            // 3. Get target velocities (from CAN commands)
            float target_vel1, target_vel2;
            bool enabled;
            get_motor_targets(&target_vel1, &target_vel2, &enabled);
            
            // 4. Apply acceleration limiting
            target_vel1 = apply_accel_limit(target_vel1, prev_target1, 
                                           MAX_ACCELERATION_RPM_S, CONTROL_PERIOD_US);
            target_vel2 = apply_accel_limit(target_vel2, prev_target2,
                                           MAX_ACCELERATION_RPM_S, CONTROL_PERIOD_US);
            
            // 5. Run PID controllers
            float output1 = pid_update(&pid1, target_vel1, vel1, CONTROL_PERIOD_US);
            float output2 = pid_update(&pid2, target_vel2, vel2, CONTROL_PERIOD_US);
            
            // 6. Safety checks
            if (!enabled || safety_fault_detected()) {
                output1 = 0.0;
                output2 = 0.0;
            }
            
            // 7. Output PWM
            set_motor_pwm(MOTOR_1, output1);
            set_motor_pwm(MOTOR_2, output2);
            
            // 8. Update telemetry (for CAN reporting)
            update_telemetry(pos1, pos2, vel1, vel2);
        }
        
        // Yield to scheduler (non-blocking)
        taskYIELD();
    }
}
```

### CAN Protocol Extensions

```
Command IDs (Pi → ESP32):
├── 0x081: MOTOR_ENABLE        [1 byte: enable flags]
├── 0x082: SET_VELOCITY        [8 bytes: float32 vel1, float32 vel2]
├── 0x083: SET_POSITION        [8 bytes: int32 pos1, int32 pos2]
├── 0x084: SET_PID_GAINS       [12 bytes: float32 Kp, Ki, Kd]
├── 0x085: EMERGENCY_STOP      [0 bytes]
├── 0x086: RESET_POSITION      [0 bytes]
├── 0x087: SET_ACCEL_LIMIT     [4 bytes: float32 accel_limit]
└── 0x091: PING                [4 bytes: uint32 timestamp] (existing)

Status IDs (ESP32 → Pi):
├── 0x101: MOTOR_STATUS        [8 bytes: uint16 flags, int16 err1, err2, current]
├── 0x102: POSITION_REPORT     [8 bytes: int32 pos1, pos2]
├── 0x103: VELOCITY_REPORT     [8 bytes: float32 vel1, vel2]
├── 0x104: FAULT_REPORT        [4 bytes: uint32 fault_flags]
├── 0x105: PID_DEBUG           [12 bytes: float32 P, I, D terms]
└── 0x111: PONG                [4 bytes: uint32 sequence] (existing)

Broadcast IDs:
├── 0x201: HEARTBEAT           [1 byte: state]
└── 0x202: EMERGENCY_BROADCAST [0 bytes]
```

### State Machine

```
┌──────────┐
│   INIT   │ ← Power-on, initialize hardware
└────┬─────┘
     │ Hardware OK
     v
┌──────────┐
│  IDLE    │ ← Waiting for commands, motors disabled
└────┬─────┘
     │ ENABLE command
     v
┌──────────┐
│ ENABLED  │ ← Motors active, accepting velocity commands
└────┬─────┘
     │ Velocity commands → Running
     │ Fault detected → FAULT
     │ DISABLE command → IDLE
     │ E-STOP → ESTOP
     v
┌──────────┐
│ RUNNING  │ ← Actively controlling motors
└────┬─────┘
     │ Velocity = 0 for timeout → ENABLED
     │ Fault → FAULT
     │ E-STOP → ESTOP
     v
┌──────────┐
│  FAULT   │ ← Error condition, motors disabled
└────┬─────┘
     │ Manual RESET → IDLE
     v
┌──────────┐
│  ESTOP   │ ← Emergency stop, requires power cycle or manual reset
└──────────┘
```

---

## File Structure for Implementation

```
esp32/src/
├── main.cpp                    # Main entry, setup, CAN communication
├── control/
│   ├── control_loop.cpp        # Main 2kHz control loop (Core 0)
│   ├── control_loop.h
│   ├── pid.cpp                 # PID controller implementation
│   ├── pid.h
│   ├── motor_control.cpp       # PWM output, direction control
│   ├── motor_control.h
│   ├── accel_limit.cpp         # Acceleration rate limiting
│   └── accel_limit.h
├── encoders/
│   ├── pcnt.cpp                # ESP32 PCNT (pulse counter) driver
│   ├── pcnt.h
│   ├── velocity_calc.cpp       # Velocity calculation from position
│   └── velocity_calc.h
├── hw/
│   ├── motor_driver.cpp        # Hardware-specific motor driver interface
│   ├── motor_driver.h
│   ├── adc_current.cpp         # Current sense ADC reading
│   └── adc_current.h
├── can/
│   ├── can_rx.cpp              # CAN message reception (Core 1)
│   ├── can_tx.cpp              # CAN message transmission
│   ├── protocol.h              # Message definitions
│   └── command_queue.cpp       # Thread-safe command queue
├── safety/
│   ├── safety.cpp              # Safety checks, fault detection
│   ├── safety.h
│   ├── estop.cpp               # Emergency stop handler
│   └── watchdog.cpp            # CAN watchdog timer
└── state/
    ├── state_machine.cpp       # System state machine
    └── shared_state.cpp        # Mutex-protected shared variables

pi/
├── motion/
│   ├── trajectory.py           # Trajectory generation
│   ├── kinematic.py            # Kinematic transformations
│   ├── planner.py              # Motion planning
│   └── limits.py               # Velocity/accel limiting
├── can/
│   ├── can_interface.py        # SocketCAN interface
│   ├── protocol.py             # Message encoding/decoding
│   ├── command_sender.py       # High-level command API
│   └── status_monitor.py       # Status reception and processing
├── gcode/
│   ├── parser.py               # G-code parsing
│   ├── interpreter.py          # G-code execution
│   └── state.py                # G-code machine state
├── ui/
│   ├── cli.py                  # Command-line interface
│   ├── web_server.py           # Flask web dashboard
│   └── dashboard.html          # Web UI
├── calibration/
│   ├── auto_tune.py            # PID auto-tuning
│   ├── kinematic_cal.py        # Kinematic calibration
│   └── step_response.py        # System identification
└── tests/
    ├── test_protocol.py        # Protocol unit tests
    ├── test_kinematics.py      # Kinematic tests
    └── integration_test.py     # End-to-end tests

protocol/
├── message_definitions.md      # Complete message catalog
├── state_transitions.md        # State machine documentation
├── timing_requirements.md      # Real-time timing specs
└── safety_spec.md              # Safety system specification
```

---

## Implementation Order & Dependencies

### Week 1: Core Infrastructure
1. Fix Pi SD card (prerequisite)
2. Complete PING/PONG (Phase 1 continuation)
3. PWM output (Phase 2.1)
4. Encoder reading (Phase 2.2)
5. Open-loop velocity (Phase 2.3)

### Week 2: Closed-Loop Control
6. PID implementation (Phase 2.4)
7. CAN velocity commands (Phase 2.5)
8. Second motor (Phase 3.1)
9. Dual motor control (Phase 3.2)

### Week 3: Position & Coordination
10. Position tracking (Phase 4.1)
11. Position control (Phase 4.2)
12. Point-to-point moves (Phase 4.3)
13. Trajectory buffer (Phase 5.1)

### Week 4: Trajectory & Safety
14. Interpolation (Phase 5.2)
15. Real-time streaming (Phase 5.3)
16. Safety systems (Phase 6.1-6.3)

### Week 5: High-Level Features
17. Calibration (Phase 7)
18. Pi-side planning (Phase 8)
19. User interface (Phase 9)

### Week 6: Testing & Validation
20. Full system integration
21. Performance testing (Phase 10)
22. Documentation completion

---

## Critical Decision Points

### Motor Driver Selection
**Options:**
1. L298N (simple, low power, 2A max)
2. DRV8833 (dual H-bridge, 1.5A continuous)
3. TB6612FNG (dual H-bridge, 1.2A, better efficiency)
4. **BTS7960 (high power, 43A, overkill but robust)** ← RECOMMENDED

**Decision:** TB6612FNG for prototype, BTS7960 for production

### Encoder Type
**Options:**
1. Optical incremental (high resolution, clean signals)
2. Magnetic incremental (robust, lower resolution)
3. Absolute encoder (no homing needed, expensive)

**Decision:** Optical incremental, 600 PPR minimum

### Control Loop Frequency
**Options:**
- 500Hz: Basic hobby projects
- **2kHz: Recommended for servo control** ← SELECTED
- 10kHz: High-performance systems, may strain ESP32

**Trade-offs:**
- Higher frequency = better response, higher CPU load
- 2kHz gives 500μs loop time, plenty for our needs

### Communication Protocol
**Options:**
1. Binary (efficient, harder to debug)
2. ASCII (human-readable, inefficient)
3. **Mixed: Binary for real-time, ASCII for config** ← SELECTED

**Decision:** Binary for motion commands, ASCII for diagnostics

---

## Risk Assessment

### High-Risk Items
1. **Pi SD card failure** (CURRENT BLOCKER)
   - Mitigation: Replace SD card, use industrial-grade
   
2. **Real-time deadline misses**
   - Mitigation: Profiling, optimization, Core 0 isolation
   
3. **CAN bus overload**
   - Mitigation: Rate limiting, prioritization
   
4. **Motor current overload**
   - Mitigation: Current sensing, software limits

### Medium-Risk Items
5. **PID instability**
   - Mitigation: Conservative tuning, anti-windup
   
6. **Encoder noise**
   - Mitigation: Hardware filtering, debouncing
   
7. **Thermal issues**
   - Mitigation: Heatsinks, thermal monitoring

### Low-Risk Items
8. **Memory exhaustion**
   - Mitigation: Static allocation, profiling
   
9. **Power supply noise**
   - Mitigation: Separate supplies, filtering

---

## Testing Strategy

### Unit Tests (Per Component)
- PID controller: Step response, setpoint tracking
- Encoder: Forward/reverse, overflow handling
- PWM: Duty cycle accuracy, frequency measurement
- Safety: Fault injection, recovery testing

### Integration Tests
- Motor control: Open-loop → closed-loop → dual motor
- CAN communication: All message types, error handling
- State machine: All transitions, fault conditions

### System Tests
- Path accuracy: Square, circle, figure-8 patterns
- Velocity accuracy: Constant velocity tracking
- Acceleration limits: Ramp testing
- Thermal: 1-hour continuous run
- Stability: 24-hour reliability test

---

## Success Criteria

### Phase 2 (Single Motor):
- [ ] Motor spins at commanded velocity ±2 RPM
- [ ] Response time < 100ms to 90% of setpoint
- [ ] No overshoot with proper tuning
- [ ] Stable at all velocities 0-120 RPM

### Phase 3 (Dual Motor):
- [ ] Independent control of both motors
- [ ] Synchronized start/stop < 10ms
- [ ] Differential drive: straight line ±5mm over 1m

### Phase 4 (Position):
- [ ] Position accuracy ±0.5mm
- [ ] Repeatability ±0.2mm
- [ ] Point-to-point move time optimized

### Phase 5 (Trajectory):
- [ ] Smooth path following (no jerking)
- [ ] Corner velocity maintained
- [ ] Buffer never underruns at 100Hz

### Phase 6 (Safety):
- [ ] E-stop response < 10ms
- [ ] No motion after fault condition
- [ ] Current limit never exceeded
- [ ] Watchdog activates on timeout

---

## Documentation Requirements

### For Each Phase:
1. Planning document (thoughtprocesses/)
2. Implementation code (with version tags)
3. Test results (test_results/)
4. Known issues (ISSUES.md)
5. Tuning parameters (TUNING.md)

### Final Deliverables:
1. Complete system architecture diagram
2. User manual (operation, G-code reference)
3. Developer guide (code structure, API)
4. Calibration procedure
5. Troubleshooting guide
6. Performance specifications
7. Safety certification documents

---

## Overnight Run Objectives

### Primary Tasks:
1. ✅ Create this comprehensive planning document
2. ⏭️ Design complete CAN protocol (all message types)
3. ⏭️ Create ESP32 file structure (headers, stubs)
4. ⏭️ Design Pi-side architecture
5. ⏭️ Write PID controller implementation
6. ⏭️ Design state machine (full specification)
7. ⏭️ Create test plan for each phase
8. ⏭️ Document all safety requirements

### Stretch Goals:
9. ⏭️ Implement encoder reading code
10. ⏭️ Implement PWM control code
11. ⏭️ Create protocol validation tests
12. ⏭️ Design calibration procedures

---

**Status:** Planning document created, ready for detailed design phase
**Next Agent:** Continue with CAN protocol design and file structure creation
