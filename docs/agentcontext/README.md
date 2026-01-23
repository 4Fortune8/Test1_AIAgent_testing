# Agent Context Directory

This directory contains metadata and tracking information for agent-driven development.

## Contents

### Core State Tracking
- **CURRENT_STATE.md**: **[PRIMARY HANDOFF DOCUMENT]** Complete project state for agent handoffs
  - Current version, phase status, completed work
  - Hardware configuration and access details
  - Next steps and immediate actions
  - Known issues and critical file locations
  - **Status:** V1.03.01 - Phase 1 Complete, Ready for Phase 2

- **version_log.md**: Project-wide changelog of all agent modifications
  - Tracks version numbers, user requests, files modified, and planning docs
  - Updated after major changes or task completion
  - Provides audit trail and context for future work

### Implementation Planning
- **roadmap_can_ping_pong.md**: **[ACTIVE ROADMAP]** Detailed implementation plan for establishing CAN PING/PONG communication
  - Status: Phase 1 Complete - Ready for Phase 2 (TWAI Driver)
  - Hardware: ESP32-WROOM-32D (GPIO5/4) + Raspberry Pi 3B+ (MCP2515)
  - Date: 2026-01-22 (Updated with GPIO5/4 configuration)
  - Scope: Minimal CAN communication bring-up (no motor control yet)

- **hardware_wiring_guide.md**: Complete physical wiring instructions
  - ESP32 GPIO5 (TX) / GPIO4 (RX) to SN65HVD230 transceiver
  - Raspberry Pi SPI (GPIO10/9/11/8/25) to MCP2515 transceiver
  - CAN bus termination, power, and troubleshooting

- **checklist_can_ping_pong.md**: Implementation tracking checklist
  - Phase-by-phase task list
  - Hardware approval status
  - Testing and validation checkboxes

- **protocol_amendment_pong.md**: PONG command specification (APPROVED)
  - CAN ID: 0x111 (STATUS class)
  - Payload: 4-byte uint32 sequence counter
  - Rationale and implementation details

### Planning Documents
Planning documents are stored in domain-specific thoughtprocesses folders:
- **`esp/thoughtprocesses/`**: ESP32 firmware planning documents
- **`pi/thoughtprocesses/`**: Raspberry Pi software planning documents  
- **`protocol/thoughtprocesses/`**: Protocol changes and additions
- **`docs/thoughtprocesses/`**: Documentation and workspace planning

Current planning documents:
- `esp/thoughtprocesses/2026-01-22_hello-world-verify.md` - Phase 1 hello world
- `docs/thoughtprocesses/2026-01-22_pre-phase2-cleanup.md` - Workspace cleanup
- `docs/thoughtprocesses/2026-01-22_workspace-audit-report.md` - File structure audit

## Active Roadmaps

### CAN PING/PONG Communication
**Document:** `roadmap_can_ping_pong.md`  
**Status:** Planning Complete - Ready for Physical Wiring  
**Current Hardware Configuration:**
- **ESP32:** GPIO5 (TX, Pin 29) / GPIO4 (RX, Pin 26) - Safe defaults, no boot conflicts
- **Pi:** MCP2515 on SPI0 (MOSI=GPIO10, MISO=GPIO9, SCLK=GPIO11, CS=GPIO8, INT=GPIO25)
- **CAN:** 500 kbps, 120Ω termination at both ends, shared ground required

**Purpose:** Establish minimal, robust CAN communication between Raspberry Pi and ESP32

**Critical Requirements:**
- All planning documents MUST be created BEFORE code implementation
- ESP Core 0 must remain isolated (control loop only)
- CAN RX/TX must run on ESP Core 1 exclusively  
- No blocking calls on ESP
- Follow AGENTS.md hard rules strictly

**Hardware Status (COMPLETE):**
1. ✅ Physical wiring complete per `hardware_wiring_guide.md`
2. ✅ Pi `can0` interface operational (UP, LOWER_UP)
3. ✅ ESP32 hello world verified (Phase 1 complete)
4. ✅ GPIO5/4 confirmed safe and readable
5. ✅ Serial communication working (115200 baud)

**Protocol Decisions (APPROVED):**
1. ✅ PING command ID: 0x091 (COMMAND_ACK class, Pi sends)
2. ✅ PONG command ID: 0x111 (STATUS class, ESP32 responds)
3. ✅ PONG payload: 4-byte uint32 sequence counter (little-endian)
4. ✅ CAN bitrate: 500 kbps
5. ✅ RTT threshold: <50ms target, 100ms max acceptable

**Next Steps:**
1. Create Phase 2 planning document (`esp/thoughtprocesses/2026-01-22_can-bringup-esp.md`)
2. Implement TWAI driver per roadmap Phase 3
3. Test CAN transmission from ESP32
4. Implement Pi CAN receiver
5. Verify PING/PONG round-trip

## Purpose

This directory helps:
- Track evolution of the codebase over time
- Maintain context across agent sessions
- Provide humans with a clear history of what changed and why
- Enable agents to understand past decisions before making new changes
- Store comprehensive roadmaps for complex features

## Usage

Before starting work, agents should:
1. Read `version_log.md` to understand recent changes
2. Check for active roadmaps in this directory
3. If implementing a roadmap feature, follow the roadmap exactly
4. Use the latest version number to determine next version tag
5. After completing work, add new entry to version log

Humans can:
- Review the version log to understand project history
- Reference version tags in code to trace back to original requests
- Use this as a basis for rollback or debugging decisions
- Review and approve roadmaps before implementation begins
