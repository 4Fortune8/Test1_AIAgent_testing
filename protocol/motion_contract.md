# Motion Contract (Pi ↔ ESP32)

## Purpose
This document defines the strict division of responsibility between
the Raspberry Pi (planner) and ESP32 (real-time controller).

This contract is binding. Violations are bugs.

---

## Raspberry Pi Responsibilities (Non-Real-Time)

The Raspberry Pi MAY:
- Perform motion planning
- Generate trajectories
- Enforce global limits
- Issue high-level motion commands
- Visualize and log motion

The Raspberry Pi MUST NOT:
- Perform encoder counting
- Run PID loops
- Generate PWM values
- Perform real-time safety enforcement
- Assume deterministic timing

The Pi sends **intent**, not implementation.

---

## ESP32 Responsibilities (Real-Time)

The ESP32 MUST:
- Decode quadrature encoders
- Run velocity and position PID loops
- Enforce acceleration limits
- Enforce safety timeouts and fault handling
- Generate PWM and direction signals
- Maintain safe operation if the Pi disconnects

The ESP32 owns:
- Encoder resolution
- PID gains
- Control loop timing
- Safety thresholds

---

## Motion Command Model

Motion commands from the Pi to the ESP32 are expressed ONLY as:

- Target position (millimeters)
- Target velocity (mm/s)
- Acceleration limit (mm/s²)
- Enable / Disable commands

The Pi MUST NOT:
- Specify PWM duty cycles
- Specify encoder tick counts
- Specify PID gains
- Override ESP safety decisions

---

## Timing Guarantees

- ESP control loop runs at a fixed frequency (1–5 kHz)
- CAN reception must never block the control loop
- Motion commands update target values atomically

---

## Error Handling

- The ESP32 may reject commands it cannot execute safely
- The ESP32 reports faults via CAN ERROR messages
- The Pi must log faults, not blindly retry

---

## Versioning

This contract is versioned implicitly by the repository.
Changes require explicit review and agreement.
