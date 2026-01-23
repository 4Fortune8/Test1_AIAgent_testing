# Distributed Control System - ESP32 & Raspberry Pi

A distributed control system implementing real-time motor control and CAN communication between ESP32 microcontrollers and a Raspberry Pi coordinator.

## Project Status

**Current Version:** V1.03.01  
**Phase:** Phase 1 Complete - Hello World Verified  
**Next:** Phase 2 - TWAI CAN Driver Implementation

### Quick Links
- 📋 **[Current State](docs/agentcontext/CURRENT_STATE.md)** - Latest project status and next steps
- 🗺️ **[Roadmap](docs/agentcontext/roadmap_can_ping_pong.md)** - CAN PING/PONG implementation plan
- 📝 **[Version Log](docs/agentcontext/version_log.md)** - Complete change history
- ⚙️ **[Agent Rules](AGENTS.md)** - Rules for AI agents working on this project

---

## System Architecture

### Raspberry Pi (High-Level Controller)
- **Role:** Orchestration, UI, networking, configuration
- **Language:** Python
- **CAN Interface:** MCP2515 via SPI (SocketCAN `can0`)
- **Responsibilities:**
  - Command generation and trajectory planning
  - User interfaces (CLI, web)
  - Logging and state management
  - Non-real-time decision making

### ESP32 (Real-Time Controller)
- **Role:** Real-time motor control, sensors, safety-critical tasks
- **Language:** C++ (Arduino framework + ESP-IDF)
- **CAN Interface:** Built-in TWAI peripheral (GPIO5 TX, GPIO4 RX)
- **Responsibilities:**
  - Motor PWM control
  - Encoder reading
  - PID control loops
  - Real-time safety monitoring
  - Deterministic timing

### Communication
- **Protocol:** CAN bus (500 kbps)
- **Style:** Command-based (Pi sends intent, ESP executes and reports status)
- **Reliability:** ESP must operate safely if Pi disconnects

---

## Project Structure

```
.
├── esp32/              # PlatformIO project (ACTIVE)
│   ├── platformio.ini  # Build configuration
│   ├── src/            # ESP32 source code
│   └── include/        # ESP32 headers
├── esp/                # Legacy/reference code
│   └── thoughtprocesses/  # Planning documents
├── pi/                 # Raspberry Pi Python code
│   ├── can/            # CAN communication
│   └── planner/        # Motion planning
├── protocol/           # Protocol specifications
│   ├── can_id_map.md   # CAN ID allocations
│   ├── can_invariants.md  # Protocol rules
│   └── can_payload.md  # Message formats
├── docs/               # Documentation
│   ├── agentcontext/   # Agent handoff docs
│   └── thoughtprocesses/  # General planning docs
└── AGENTS.md           # Rules for AI agents

```

---

## Hardware Configuration

### ESP32-WROOM-32D (UPeasy DevKit)
- **CAN TX:** GPIO5 (Pin 29) → SN65HVD230 transceiver
- **CAN RX:** GPIO4 (Pin 26) → SN65HVD230 transceiver
- **Transceiver:** SN65HVD230 (3.3V, RS pin grounded for high-speed)
- **Power:** USB or external 5V
- **Termination:** 120Ω resistor installed

### Raspberry Pi 3B+
- **CAN Interface:** MCP2515 SPI module
  - MOSI: GPIO10
  - MISO: GPIO9
  - SCLK: GPIO11
  - CS: GPIO8
  - INT: GPIO25
- **SocketCAN:** `can0` interface (500 kbps)
- **Access:** SSH `fabricpi@10.42.0.48` (password: fabricpi)
- **Termination:** 120Ω resistor installed

### CAN Bus
- **Bitrate:** 500 kbps
- **Topology:** Point-to-point (Pi ↔ ESP32)
- **Termination:** 120Ω at both ends (~60Ω measured)
- **Ground:** Shared between Pi and ESP32

Complete wiring instructions: [Hardware Wiring Guide](docs/agentcontext/hardware_wiring_guide.md)

---

## Getting Started

### ESP32 Development

```bash
# Navigate to PlatformIO project
cd esp32

# Build firmware
~/.platformio/penv/bin/pio run

# Upload to ESP32
~/.platformio/penv/bin/pio run --target upload

# Monitor serial output (115200 baud)
~/.platformio/penv/bin/pio device monitor --baud 115200
```

**Note:** Use `~/.platformio/penv/bin/pio` (v6.1.18), not `/usr/bin/pio` (v4.3.4 obsolete)

### Raspberry Pi Development

```bash
# SSH to Pi
ssh fabricpi@10.42.0.48

# Check CAN interface
ip link show can0

# Monitor CAN traffic
candump can0

# Send test frame
cansend can0 123#DEADBEEF
```

---

## Development Workflow

### For AI Agents
1. **Read AGENTS.md first** - Contains all hard rules
2. **Check CURRENT_STATE.md** - Understand current phase
3. **Create planning document** before coding (in appropriate `thoughtprocesses/` folder)
4. **Follow version numbering** - Major increments require user approval
5. **Update version_log.md** after completing work

### For Humans
1. Review **CURRENT_STATE.md** for latest status
2. Check **version_log.md** for recent changes
3. Follow **roadmap_can_ping_pong.md** for implementation plan
4. Use **checklist_can_ping_pong.md** to track progress

---

## Key Principles

### Hard Rules (Never Violate)
1. **NO blocking delays on ESP firmware** (except `setup()`)
2. **NO real-time logic on Raspberry Pi**
3. **NO protocol changes without explicit request**
4. **NO style refactoring of working hardware code**
5. **NO assumptions about hardware availability**

### Architecture
- **Determinism:** ESP must be predictable and reliable
- **Fault Tolerance:** ESP must operate safely if Pi disconnects
- **Explicit Communication:** Command-based, not shared state
- **Clear Ownership:** Each component has defined responsibilities

### Safety
- ESP validates all incoming commands
- ESP never trusts Pi timing
- Destructive actions must be guarded
- Hardware may be disconnected at any time

---

## Current Phase: CAN PING/PONG

### Goal
Establish minimal, robust CAN communication between Pi and ESP32.

### Completed (Phase 1)
- ✅ Hardware wiring and verification
- ✅ Pi `can0` interface operational
- ✅ ESP32 hello world firmware verified
- ✅ GPIO5/4 confirmed safe
- ✅ Protocol specified (PING=0x091, PONG=0x111)

### Next (Phase 2)
- [ ] Implement ESP32 TWAI driver
- [ ] Send test frames from ESP32
- [ ] Receive frames on Pi
- [ ] Implement PING/PONG protocol
- [ ] Measure and optimize RTT

Target RTT: <50ms average, <100ms maximum

---

## Documentation

### Agent Context
- [CURRENT_STATE.md](docs/agentcontext/CURRENT_STATE.md) - Primary handoff document
- [version_log.md](docs/agentcontext/version_log.md) - Complete change history
- [README.md](docs/agentcontext/README.md) - Agent context index

### Implementation
- [roadmap_can_ping_pong.md](docs/agentcontext/roadmap_can_ping_pong.md) - Phase-by-phase plan
- [checklist_can_ping_pong.md](docs/agentcontext/checklist_can_ping_pong.md) - Task tracking
- [hardware_wiring_guide.md](docs/agentcontext/hardware_wiring_guide.md) - Physical setup

### Protocol
- [can_id_map.md](protocol/can_id_map.md) - CAN ID allocations
- [can_invariants.md](protocol/can_invariants.md) - Protocol rules (15 binding)
- [can_payload.md](protocol/can_payload.md) - Message formats
- [protocol_amendment_pong.md](docs/agentcontext/protocol_amendment_pong.md) - PONG spec

### System Design
- [roles.md](docs/roles.md) - Component responsibilities
- [timing_constraints.md](docs/timing_constraints.md) - Real-time requirements
- [can_assumptions.md](docs/can_assumptions.md) - CAN design decisions

---

## License

[Specify license here]

## Contact

[Specify contact/maintainer here]

---

**Last Updated:** 2026-01-22 (V1.03.02)
