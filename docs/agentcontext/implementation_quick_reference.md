# Quick Implementation Reference

**Version:** V2.00.00  
**For:** Developers implementing the motor control system  
**See Also:** docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md

---

## File Structure

```
esp32/
├── include/
│   ├── can_protocol.h       - CAN message handling
│   ├── motor_control.h      - Real-time control (Core 0)
│   ├── encoder.h            - Quadrature encoders (PCNT)
│   ├── motor_driver.h       - PWM motor drivers
│   └── pid_controller.h     - PID controller
├── src/
│   ├── main.cpp             - Dual-core startup
│   ├── can_protocol.cpp     - [TODO] CAN protocol implementation
│   ├── motor_control.cpp    - [TODO] Control loop implementation
│   ├── encoder.cpp          - [TODO] Encoder implementation
│   ├── motor_driver.cpp     - [TODO] Motor driver implementation
│   └── pid_controller.cpp   - [TODO] PID implementation

pi/
├── can/
│   └── can_interface.py     - CAN communication class
├── planner/
│   └── trajectory_planner.py - Trajectory generation
└── app.py                    - [TODO] Main application

protocol/
└── motor_control_messages.md - Message format specification
```

---

## Pin Assignments (Example - Adjust to Your Hardware)

### ESP32 Pins
```cpp
// CAN
#define CAN_TX_PIN    GPIO_NUM_5
#define CAN_RX_PIN    GPIO_NUM_4

// Motor 1
#define M1_PWM        GPIO_NUM_16
#define M1_DIR1       GPIO_NUM_17
#define M1_DIR2       GPIO_NUM_18
#define M1_CURRENT    ADC1_CHANNEL_6  // GPIO34

// Motor 2
#define M2_PWM        GPIO_NUM_19
#define M2_DIR1       GPIO_NUM_21
#define M2_DIR2       GPIO_NUM_22
#define M2_CURRENT    ADC1_CHANNEL_7  // GPIO35

// Encoder 1
#define ENC1_A        GPIO_NUM_25
#define ENC1_B        GPIO_NUM_26

// Encoder 2
#define ENC2_A        GPIO_NUM_27
#define ENC2_B        GPIO_NUM_14

// Emergency stop (optional)
#define ESTOP_PIN     GPIO_NUM_23
```

---

## Core Allocation

### Core 0 (Real-Time)
```cpp
void control_loop_task(void* parameter) {
    while (1) {
        uint32_t start_time = micros();
        
        motor_control_loop();  // 2kHz control loop
        
        uint32_t elapsed = micros() - start_time;
        if (elapsed < CONTROL_LOOP_PERIOD_US) {
            delayMicroseconds(CONTROL_LOOP_PERIOD_US - elapsed);
        }
    }
}

// In setup():
xTaskCreatePinnedToCore(
    control_loop_task,
    "ControlLoop",
    4096,
    NULL,
    1,  // Priority
    NULL,
    0   // Core 0
);
```

### Core 1 (Communication)
```cpp
void can_task(void* parameter) {
    while (1) {
        // Non-blocking CAN RX/TX
        can_protocol_process();
        vTaskDelay(pdMS_TO_TICKS(1));  // 1ms = 1kHz
    }
}

// In setup():
xTaskCreatePinnedToCore(
    can_task,
    "CanComm",
    4096,
    NULL,
    1,
    NULL,
    1   // Core 1
);
```

---

## CAN Message Quick Reference

### Commands (Pi → ESP32)
```python
# Enable motors
can_if.send_motor_enable(motor1=True, motor2=True)

# Set velocity (RPM)
can_if.send_set_velocity(motor1_rpm=60.0, motor2_rpm=60.0)

# Set position (encoder counts)
can_if.send_set_position(motor1_pos=10000, motor2_pos=10000)

# Emergency stop
can_if.send_emergency_stop()

# Clear fault
can_if.send_clear_fault()

# Set PID gains (TODO: multi-frame)
# can_if.send_set_pid_gains(motor_id=0, kp=1.0, ki=0.1, kd=0.01)
```

### Status (ESP32 → Pi)
```python
# Register callbacks
def on_position(msg):
    pos = can_if.get_position()
    print(f"M1: {pos.motor1_pos}, M2: {pos.motor2_pos}")

can_if.register_callback(CANStatus.POSITION_REPORT, on_position)

# Or poll latest values
state = can_if.get_system_state()  # SystemState enum
position = can_if.get_position()    # MotorPosition dataclass
velocity = can_if.get_velocity()    # MotorVelocity dataclass
faults = can_if.get_fault_flags()   # uint32_t bit field
```

---

## PID Tuning Guide

### Initial Tuning (Ziegler-Nichols)
```cpp
// Step 1: Set Ki=0, Kd=0, increase Kp until oscillation
pid_set_gains(&motor1_pid, 2.0, 0.0, 0.0);

// Step 2: Measure critical gain Ku and period Tu
// Ku = gain at sustained oscillation
// Tu = oscillation period in seconds

// Step 3: Apply Ziegler-Nichols
float kp = 0.6 * Ku;
float ki = 2.0 * kp / Tu;
float kd = kp * Tu / 8.0;

pid_set_gains(&motor1_pid, kp, ki, kd);
```

### Manual Fine-Tuning
```
Too much overshoot?  → Decrease Kp
Too slow?            → Increase Kp
Steady-state error?  → Increase Ki
Oscillation?         → Decrease Kp, increase Kd
Noisy?               → Decrease Kd
```

---

## Trajectory Planning Examples

### Simple Point-to-Point
```python
planner = TrajectoryPlanner(num_axes=2)

start = np.array([0, 0])
end = np.array([10000, 5000])

trajectory = planner.plan_point_to_point(start, end)

# Send to ESP32 via CAN
for point in trajectory.points:
    # TODO: Implement CAN message encoding
    pass
```

### Multi-Waypoint Path
```python
waypoints = [
    np.array([0, 0]),
    np.array([10000, 0]),
    np.array([10000, 10000]),
    np.array([0, 10000]),
    np.array([0, 0])
]

path = planner.plan_path(waypoints)
print(f"Total path time: {path.duration:.2f}s")
```

### Time-Optimal Move
```python
optimal = planner.optimize_time(start, end)
print(f"Fastest time: {optimal.duration:.2f}s")
```

---

## State Machine Transitions

```
Valid transitions:
  INIT → IDLE          (always on boot)
  IDLE → ENABLED       (via MOTOR_ENABLE)
  ENABLED → RUNNING    (via SET_VELOCITY/SET_POSITION)
  RUNNING → ENABLED    (motors stop naturally)
  ENABLED → IDLE       (via MOTOR_ENABLE disable)
  
  Any → FAULT          (on error detection)
  Any → ESTOP          (via EMERGENCY_STOP)
  
  FAULT → IDLE         (via CLEAR_FAULT)
  ESTOP → INIT         (via power cycle only)

Invalid transitions (will be rejected):
  IDLE → RUNNING       (must enable first)
  FAULT → RUNNING      (must clear fault)
  ESTOP → anything     (must reset)
```

---

## Common Debug Commands

### ESP32 Serial Monitor
```
[HEARTBEAT] State: RUNNING, Uptime: 123s
[MOTOR] M1: pos=1234, vel=60.5rpm, cur=1.2A
[MOTOR] M2: pos=5678, vel=59.8rpm, cur=1.1A
[PID] M1: P=0.45, I=0.02, D=0.01, Out=0.48
[TIMING] Avg: 385us, Max: 423us, Overruns: 0
```

### Pi CAN Monitoring
```bash
# View all CAN traffic
candump can0

# View specific message
candump can0,102:7FF  # Position reports only

# Send test command
cansend can0 081#01  # Enable motor 1
```

---

## Safety Checklist

Before running motors:
- [ ] Emergency stop button accessible
- [ ] Current limits configured correctly
- [ ] Position limits set (if applicable)
- [ ] Encoders verified connected
- [ ] Motor driver wiring correct
- [ ] Power supply adequate for load
- [ ] Mechanical stops/guards in place
- [ ] Test emergency stop response
- [ ] Verify watchdog timeout works
- [ ] Review fault recovery procedures

---

## Performance Targets

| Metric | Target | Critical? |
|--------|--------|-----------|
| Control loop rate | 2000 Hz | Yes |
| Control loop jitter | < 50 μs | Yes |
| Emergency stop latency | < 1 ms | **Critical** |
| CAN PING/PONG RTT | < 5 ms | No |
| Position report rate | 50 Hz | No |
| Velocity control error | < 1% | No |
| Position control error | < 100 counts | No |
| Trajectory following error | < 100 counts | No |

---

## Troubleshooting

### Motors not moving
1. Check system state (should be RUNNING)
2. Verify motors enabled via CAN
3. Check PWM output with oscilloscope
4. Verify power supply voltage
5. Check motor driver enable pins

### Encoders not counting
1. Check PCNT configuration
2. Verify encoder wiring (A, B, GND)
3. Test with manual rotation
4. Check for loose connections
5. Verify encoder power (if required)

### CAN communication failed
1. Check bus termination (120Ω both ends)
2. Verify CAN H/L wiring
3. Check bitrate matches (500kbps)
4. Use oscilloscope to verify signals
5. Check for common ground

### Control loop timing issues
1. Profile code with esp_timer_get_time()
2. Check for blocking calls in Core 0
3. Verify task priorities
4. Disable logging in control loop
5. Optimize hot paths

### Oscillation in PID controller
1. Reduce Kp gain
2. Increase Kd gain
3. Check for mechanical backlash
4. Verify encoder resolution sufficient
5. Add velocity filtering

---

## Testing Order

1. **Hardware verification** - Pins, wiring, power
2. **Motor driver** - PWM output, direction control
3. **Encoders** - Count accuracy, velocity measurement
4. **Single motor PID** - Velocity control
5. **Dual motor** - Synchronized motion
6. **CAN protocol** - All message types
7. **State machine** - All transitions
8. **Safety** - Emergency stop, faults, watchdog
9. **Trajectory** - Path following
10. **Integration** - End-to-end scenarios

---

## Where to Start

### Day 1: Motor Driver
1. Implement `motor_driver.cpp`
2. Test PWM output with oscilloscope
3. Verify direction control
4. Measure current sensor

### Day 2: Encoders
1. Implement `encoder.cpp` with PCNT
2. Test position counting
3. Verify velocity calculation
4. Test at various speeds

### Day 3: PID Controller
1. Implement `pid_controller.cpp`
2. Unit test with simulated system
3. Integrate with motor driver
4. Test single motor velocity control

### Day 4-5: Dual Motor System
1. Implement `motor_control.cpp`
2. Set up dual-core architecture
3. Test real-time control loop timing
4. Verify dual motor coordination

### Day 6-7: CAN Protocol
1. Implement `can_protocol.cpp`
2. Test all message types
3. Measure latency and throughput
4. Integrate with motor control

### Week 2+: Continue through remaining phases...

---

**Quick Links:**
- Master Plan: `docs/thoughtprocesses/2026-01-22_overnight-motor-control-planning.md`
- Protocol Spec: `protocol/motor_control_messages.md`
- Testing: `docs/agentcontext/testing_strategy.md`
- Summary: `docs/thoughtprocesses/2026-01-22_overnight-planning-summary.md`
