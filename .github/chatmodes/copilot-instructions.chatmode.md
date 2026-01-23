# AGENTS.md – Overnight Planning Run Mode

**CURRENT MODE: OVERNIGHT PLANNING RUN**  
**Branch: overnightRun**  
**Objective: Complete CNC Motor Control System Design**
**CHECK REGULARLY FOR UPDATES. Ebb and flow with the project's needs.**
## Active Mission

Plan and document a complete motor control system for CNC-style wheel movement. This is a PLANNING run - focus on design, documentation, and architecture, NOT implementation on hardware.

### Overnight Run Goals
1. ✅ Master planning document created
2. Design complete CAN protocol (all 20+ message types)
3. Create full ESP32 file structure with headers
4. Design Pi-side architecture  
5. Write PID controller implementation (code)
6. Design complete state machine
7. Create comprehensive test plans
8. Document all safety requirements
9. Design calibration procedures
10. Create protocol validation framework

### Rules for This Run
- **PLANNING ONLY** - No hardware changes
- **DOCUMENT EVERYTHING** - Every decision needs rationale
- **BE THOROUGH** - This is the blueprint for months of work
- **CHECK YOURSELF** - Review each design for consistency
- **CROSS-REFERENCE** - Link all related documents
- **THINK LONG-TERM** - Consider maintenance, debugging, upgrades

---

## Agent Rules (READ FIRST)

**Meta-Rule: When editing this file, use minimal words. Be concise.**

### Hard Rules
- **DO NOT introduce blocking delays on ESP firmware**
- **DO NOT move real-time logic to the Raspberry Pi**
- **DO NOT change communication protocol formats unless explicitly requested**
- **DO NOT refactor working hardware code for "style"**
- **DO NOT assume hardware availability when writing Pi-side code**
- Prefer **small, localized changes** over large refactors
- Prefer **modifying existing files** over creating new ones
- Agents must follow the role definitions in docs/roles.md.
- Any violation is a bug.

If unsure, ask before making architectural changes, if the user asks for a request too large in scope ask to narrow the reqest down.

### Planning & Documentation Requirements

**BEFORE writing or modifying any code:**

1. **Create a thought process folder** for the relevant directory:
   - For ESP changes: `esp/thoughtprocesses/`
   - For Pi changes: `pi/thoughtprocesses/`
   - For protocol changes: `protocol/thoughtprocesses/`
   - For documentation: `docs/thoughtprocesses/`

2. **Create a planning document** in the thought process folder:
   - Name format: `YYYY-MM-DD_feature-name.md` or `YYYY-MM-DD_issue-description.md`
   - Include:
     - **User request**: The original request verbatim from the user
     - **Problem statement**: What are we trying to solve?
     - **Constraints**: What rules/limits apply? (real-time, safety, protocol, etc.)
     - **Approach**: Step-by-step plan
     - **Files to modify**: List of specific files and why
     - **Risks**: What could go wrong?
     - **Testing strategy**: How will we verify it works?

3. **Share the plan** before implementing:
   - Allow human review of the approach
   - Confirm alignment with architecture rules
   - Verify no hard rules are violated

4. **Reference the plan in code**:
   - Add comments in modified code referencing the planning document
   - **DO NOT overwrite existing context comments** - add new versioned comments instead
   - Use version tags (V1.00.00, V1.00.01, V1.01.00, etc.) to track evolution of code sections
   - Format: `// V{major}.{step}.{query}: User request: "..." | Plan: path/to/plan.md`
   - Version components:
     - **Major**: Significant feature/architectural change (V1.xx.xx → V2.xx.xx)
     - **Step**: Sub-task within major change (V1.00.xx → V1.01.xx)
     - **Query**: Individual interaction index (V1.00.01 → V1.00.02)
   - When modifying existing code, add a new versioned comment above
   - Example:
     ```cpp
     // V1.00.01: User request: "Add emergency stop" | Plan: esp/thoughtprocesses/2026-01-22_emergency-stop.md
     void emergencyStop() { /* ... */ }
     
     // V1.02.05: User request: "Make e-stop trigger safety LED" | Plan: esp/thoughtprocesses/2026-01-25_safety-led.md
     void emergencyStop() { 
       stopMotors();
       activateSafetyLED();  // V1.02.05 addition
     }
     ```

5. **Update version log**:
   - After completing major changes or tasks, update `docs/agentcontext/version_log.md`
   - Log should include: version number, date, user request summary, files modified, planning doc reference
   - This creates a project-wide changelog of agent modifications

### Version Numbering Rules

Format: `V{major}.{step}.{query}`

**Major (V1→V2): USER APPROVAL REQUIRED**
- Architectural changes, protocol changes, major features
- Agent MUST ask before incrementing

**Step (V1.00→V1.01): Agent discretion**
- Sub-features, refactors, configuration updates

**Query (V1.00.00→V1.00.01): Automatic**
- Individual interactions, fixes, documentation

Most work = step/query increments. When unsure, use lower level.

**Benefits:**
- Humans can catch architectural issues early
- Future agents can understand past decisions
- Creates a searchable history of design rationale
- Reduces rework and bugs

**Example:**
```
Before adding a new sensor:
1. Create esp/thoughtprocesses/2026-01-22_add-temperature-sensor.md
2. Document: 
   - User request: "Add DHT22 temperature sensor to monitor ambient conditions"
   - Problem: Need environmental monitoring for thermal management
   - Sensor type, pin assignment, sampling rate, integration approach
3. Wait for confirmation
4. Implement according to plan
5. Add code comments: // V1.00.01: User request: "Add DHT22 temp sensor" | Plan: esp/thoughtprocesses/2026-01-22_add-temperature-sensor.md
6. Update docs/agentcontext/version_log.md with the completed change
```P Controller

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

If unsure, ask before making architectural changes, if the user asks for a request too large in scope ask to narrow the reqest down.

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
