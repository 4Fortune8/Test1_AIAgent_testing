# Testing Strategy - Motor Control System

**Date:** 2026-01-22  
**Version:** V2.00.00  
**Parent Plan:** docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md

---

## Overview

Comprehensive testing strategy for the CNC-style motor control system. Testing is organized into phases that align with the implementation phases.

---

## Testing Philosophy

1. **Test early, test often** - Unit tests written before or during implementation
2. **Hardware in the loop** - Real hardware testing at every phase
3. **Safety first** - Always test safety features thoroughly
4. **Incremental validation** - Each phase builds on tested previous phases
5. **Automated where possible** - CI/CD for software tests, scripts for hardware

---

## Phase 0: CAN Communication Foundation

### Unit Tests (Pi)
- [ ] `test_can_interface.py`
  - Message encoding/decoding
  - Callback registration
  - Watchdog timeout detection
  - Statistics tracking

### Unit Tests (ESP32)
- [ ] `test_can_protocol.cpp`
  - Message parsing
  - Invalid message rejection
  - Sequence number handling
  - Callback dispatch

### Integration Tests
- [ ] PING/PONG round-trip time measurement
- [ ] Message loss under high load
- [ ] Priority arbitration (send mixed priority messages)
- [ ] Bus recovery from error states

### Hardware Tests
- [ ] Physical CAN bus with termination
- [ ] Signal integrity (oscilloscope verification)
- [ ] Maximum message rate sustained
- [ ] Hot-plug tolerance

**Success Criteria:**
- RTT < 5ms for PING/PONG
- No message loss at 100 Hz command rate
- Bus recovery within 100ms

---

## Phase 1: Motor Driver Hardware

### Unit Tests (ESP32)
- [ ] `test_motor_driver.cpp`
  - PWM frequency verification
  - Duty cycle accuracy (0%, 25%, 50%, 75%, 100%)
  - Direction control logic
  - Emergency stop response time
  - Current sensor calibration

### Hardware Tests
- [ ] PWM output measurement (oscilloscope)
- [ ] Motor direction verification (both directions)
- [ ] Current sensor accuracy (compare to bench meter)
- [ ] Thermal performance (continuous operation)
- [ ] Emergency stop latency (must be < 1ms)

### Safety Tests
- [ ] Overcurrent protection triggers correctly
- [ ] Emergency stop works from all states
- [ ] Fault recovery procedures

**Success Criteria:**
- PWM frequency 20kHz ± 1%
- Duty cycle error < 2%
- Current measurement error < 5%
- Emergency stop response < 1ms

---

## Phase 2: Encoder Interface

### Unit Tests (ESP32)
- [ ] `test_encoder.cpp`
  - Position counting (forward/reverse)
  - Overflow handling (32-bit extension)
  - Velocity calculation accuracy
  - Position reset/set

### Hardware Tests
- [ ] Manual rotation (count encoder edges)
- [ ] High-speed rotation (verify no missed counts)
- [ ] Velocity measurement accuracy (compare to tachometer)
- [ ] Direction detection

### Stress Tests
- [ ] Maximum RPM without count loss
- [ ] Rapid direction changes
- [ ] Extended operation (no overflow)

**Success Criteria:**
- No missed counts up to max RPM
- Velocity error < 1 RPM
- Position tracking over millions of counts

---

## Phase 3: PID Control Loop

### Unit Tests (ESP32)
- [ ] `test_pid_controller.cpp`
  - Proportional response
  - Integral windup handling
  - Derivative filtering
  - Output limiting
  - Gain update without discontinuity

### Simulation Tests
- [ ] Step response (ideal system model)
- [ ] Disturbance rejection
- [ ] Setpoint tracking
- [ ] Parameter tuning convergence

### Hardware Tests (Single Motor)
- [ ] Velocity control at various setpoints (10, 30, 60, 120 RPM)
- [ ] Step response (measure rise time, overshoot, settling time)
- [ ] Load disturbance rejection (apply brake, measure recovery)
- [ ] Tracking error at steady state

### Tuning Tests
- [ ] Manual gain tuning procedure
- [ ] Stability margin verification
- [ ] Frequency response (Bode plot if possible)

**Success Criteria:**
- Steady-state error < 1%
- Overshoot < 10%
- Settling time < 500ms
- No oscillation at any tested setpoint

---

## Phase 4: Real-Time Control Loop

### Timing Tests
- [ ] `test_control_loop_timing.cpp`
  - Average loop time < 400μs
  - Maximum loop time < 500μs
  - Jitter < 50μs
  - No overruns over 10 minute test

### Core Affinity Tests
- [ ] Core 0 exclusively runs control loop
- [ ] Core 1 handles CAN without affecting Core 0
- [ ] Interrupt priorities correct

### Stress Tests
- [ ] Control loop + CAN RX at 100 Hz
- [ ] Control loop + CAN TX at 50 Hz
- [ ] Control loop + logging
- [ ] Control loop under WiFi traffic (if enabled)

**Success Criteria:**
- Control loop timing deterministic
- Zero overruns in 1 hour test
- CAN messages do not block control loop

---

## Phase 5: Dual Motor Coordination

### Unit Tests
- [ ] Synchronized position moves
- [ ] Coordinated velocity changes
- [ ] Independent vs synchronized modes
- [ ] Home position management

### Hardware Tests
- [ ] Both motors reach target simultaneously
- [ ] Position error between motors < 10 counts
- [ ] Coordinated acceleration/deceleration
- [ ] One motor fault doesn't destabilize other

**Success Criteria:**
- Synchronization error < 5ms
- Position matching error < 0.1%
- Independent fault isolation

---

## Phase 6: Trajectory Execution

### Unit Tests (Pi)
- [ ] `test_trajectory_planner.py`
  - Trapezoidal profile generation
  - S-curve profile generation
  - Multi-waypoint paths
  - Time-optimal planning
  - Velocity blending

### Integration Tests
- [ ] Trajectory transmission over CAN
- [ ] Buffer management (no underruns)
- [ ] Trajectory execution timing accuracy
- [ ] Pause/resume functionality

### Hardware Tests
- [ ] Execute square path
- [ ] Execute circle path (interpolated)
- [ ] Variable velocity along path
- [ ] Emergency stop during trajectory

**Success Criteria:**
- Path following error < 100 counts
- No buffer underruns
- Smooth velocity transitions
- Emergency stop works during execution

---

## Phase 7: Safety Systems

### Safety Feature Tests
- [ ] Watchdog timeout triggers safe state
- [ ] Position limit enforcement
- [ ] Velocity limit enforcement
- [ ] Current limit enforcement
- [ ] Emergency stop from all states
- [ ] Fault state recovery procedures

### Fault Injection Tests
- [ ] Encoder disconnect
- [ ] Motor driver fault
- [ ] CAN bus disconnect
- [ ] Pi crash/restart
- [ ] Power supply dropout
- [ ] Commanded impossible motion

### Safety Certification Tests
- [ ] Emergency stop response time (< 1ms)
- [ ] Fault detection time
- [ ] Safe state maintained during faults
- [ ] Cannot exit ESTOP without reset

**Success Criteria:**
- All faults detected within 10ms
- Emergency stop always < 1ms
- No unsafe states reachable
- Fault recovery documented and tested

---

## Phase 8: State Machine Validation

### State Transition Tests
- [ ] Valid transitions only
- [ ] Invalid transitions rejected
- [ ] State persistence across commands
- [ ] Recovery from each fault state

### Test Matrix
Create matrix of all state × command combinations:
- INIT: Test all commands
- IDLE: Test all commands
- ENABLED: Test all commands
- RUNNING: Test all commands
- FAULT: Test recovery commands
- ESTOP: Test only reset

**Success Criteria:**
- State diagram matches implementation
- No undefined states
- All transitions documented

---

## Phase 9: Performance Testing

### Throughput Tests
- [ ] Maximum CAN message rate
- [ ] Maximum trajectory update rate
- [ ] Maximum control loop frequency
- [ ] CPU utilization under load

### Latency Tests
- [ ] Command to actuation latency
- [ ] Fault detection to stop latency
- [ ] Status update latency

### Endurance Tests
- [ ] 24-hour continuous operation
- [ ] Thermal stability
- [ ] Memory leak detection
- [ ] Error accumulation

**Success Criteria:**
- No performance degradation over time
- No memory leaks
- Thermal equilibrium reached

---

## Phase 10: System Integration

### End-to-End Tests
- [ ] Full CNC-style motion sequence
- [ ] Multiple trajectory execution
- [ ] Homing procedure
- [ ] Calibration procedure
- [ ] Parameter save/load

### User Scenario Tests
- [ ] Power-on to first motion
- [ ] Normal operation sequence
- [ ] Fault recovery sequence
- [ ] Shutdown sequence

### Stress Tests
- [ ] Rapid command changes
- [ ] Maximum acceleration moves
- [ ] Continuous operation at limits

**Success Criteria:**
- All user scenarios work end-to-end
- No unexpected behaviors
- Documentation matches reality

---

## Regression Testing

After any code change, run:
1. **Quick smoke test** (~5 minutes)
   - CAN PING/PONG
   - Motor enable/disable
   - Single velocity command
   - Emergency stop

2. **Basic regression** (~30 minutes)
   - All unit tests
   - Motor driver tests
   - Encoder tests
   - PID step response

3. **Full regression** (~2 hours)
   - All automated tests
   - Selected hardware tests
   - Safety feature verification

---

## Test Automation

### Continuous Integration (Software)
```yaml
# .github/workflows/test.yml
on: [push, pull_request]
jobs:
  test-pi:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - run: pip install -r requirements.txt
      - run: pytest pi/tests/
      
  test-esp32:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - run: pio test
```

### Hardware Test Scripts
```bash
#!/bin/bash
# test_hardware_basic.sh

echo "Testing motor driver..."
./test_motor_pwm.py

echo "Testing encoders..."
./test_encoder_counts.py

echo "Testing CAN communication..."
./test_can_ping_pong.py

echo "All basic hardware tests passed!"
```

---

## Test Documentation

Each test should document:
- **Purpose:** What is being tested
- **Procedure:** Step-by-step instructions
- **Expected result:** What should happen
- **Actual result:** What did happen (during test)
- **Pass/fail criteria:** Clear threshold
- **Required equipment:** Oscilloscope, multimeter, etc.

---

## Safety Test Checklist

Before considering system "safe for operation":
- [ ] Emergency stop tested from all states
- [ ] Emergency stop latency measured < 1ms
- [ ] All fault conditions trigger safe state
- [ ] Watchdog timeout tested
- [ ] Overcurrent protection verified
- [ ] Position limits enforced
- [ ] Cannot enable motors in fault state
- [ ] ESTOP requires physical reset
- [ ] Fault recovery procedures documented
- [ ] Unexpected states trigger fault

---

## Test Metrics

Track over time:
- Test pass rate
- Code coverage (aim for >80%)
- Hardware test frequency
- Bugs found in testing vs production
- Time spent debugging

---

## Next Steps After Testing

Once all tests pass:
1. Document any deviations from plan
2. Update design based on test learnings
3. Create user manual
4. Create maintenance procedures
5. Begin production use with monitoring

---

**Status:** Complete test strategy  
**Next:** Begin implementation with test-driven development
