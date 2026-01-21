# AGENTS.md – Raspberry Pi ↔ ESP Controller

## Project Purpose
This repository implements a distributed control system where:
- A **Raspberry Pi** runs high-level logic, orchestration, UI, and networking.
- One or more **ESP microcontrollers** handle real-time I/O, sensors, motors, and interrupts.

The Pi is **not real-time**.
The ESP **is real-time**.

The system is designed to be:
- Deterministic on the ESP
- Fault-tolerant to Pi restarts
- Explicit in communication and state ownership

---

## Agent Rules (READ FIRST)

### Hard Rules
- **DO NOT introduce blocking delays on ESP firmware**
- **DO NOT move real-time logic to the Raspberry Pi**
- **DO NOT change communication protocol formats unless explicitly requested**
- **DO NOT refactor working hardware code for “style”**
- **DO NOT assume hardware availability when writing Pi-side code**
- Prefer **small, localized changes** over large refactors
- Prefer **modifying existing files** over creating new ones
- Agents must follow the role definitions in docs/roles.md.
- Any violation is a bug.

If unsure, ask before making architectural changes.

---

## Architecture Overview

### Raspberry Pi Responsibilities
- High-level decision making
- User interfaces (CLI, web, UI)
- Networking and remote control
- Configuration management
- Logging and long-term state

### ESP Responsibilities
- Motor control
- Sensors
- Timing-sensitive tasks
- PWM, ADC, GPIO, interrupts
- Safety-critical behavior

The ESP must be able to:
- Run safely if Pi disconnects
- Reject invalid commands
- Maintain internal failsafes

The ESP32 control loop:
- Runs on Core 0
- Must remain non-blocking
- Must not call CAN, Serial, Wi-Fi, or logging APIs
- Must not allocate memory

Any violation is a critical bug.

---

## Communication Model

- Transport: CAN 
- Direction: **Command-based**, not shared state
- Pi sends **intent**
- ESP executes and reports **status**

### Protocol Rules
- Messages must be versioned
- Backwards compatibility is preferred
- ESP validates all incoming commands
- ESP never trusts Pi timing

Do not invent new message types unless requested.

---

## Code Organization

- `/pi/` → Raspberry Pi code only
- `/esp/` → ESP firmware only
- `/protocol/` → Shared message definitions
- `/docs/` → Human documentation

Do not mix Pi and ESP logic in the same files.

---

## Language & Style Guidelines

### ESP
- Language: C++ (Arduino / ESP-IDF as specified)
- Avoid dynamic allocation in hot paths
- Avoid exceptions
- Avoid STL where possible
- Determinism > elegance

### Raspberry Pi
- Language: Python / C++ / Node (specify)
- Prefer explicit state machines
- Avoid busy loops
- Support mock / simulation modes

---

## Safety & Testing

- Assume hardware may be disconnected
- All destructive actions must be guarded
- Add logging rather than silent failure
- Prefer dry-run or simulation flags when possible

---

## Change Philosophy

This system prioritizes:
1. Predictability
2. Hardware safety
3. Clear ownership of responsibility

New features should not compromise existing stability.
