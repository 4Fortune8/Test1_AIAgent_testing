# Agent Context Directory

This directory contains metadata and tracking information for agent-driven development.

## Contents

- **version_log.md**: Project-wide changelog of all agent modifications
  - Tracks version numbers, user requests, files modified, and planning docs
  - Updated after major changes or task completion
  - Provides audit trail and context for future work

- **roadmap_can_ping_pong.md**: **[ACTIVE ROADMAP]** Detailed implementation plan for establishing CAN PING/PONG communication
  - Status: Planning Complete - Ready for Implementation
  - Date: 2026-01-22
  - Scope: Minimal CAN communication bring-up (no motor control yet)

## Active Roadmaps

### CAN PING/PONG Communication
**Document:** `roadmap_can_ping_pong.md`  
**Status:** Planning Complete - Awaiting Human Review  
**Purpose:** Establish minimal, robust CAN communication between Raspberry Pi and ESP32

**Critical Requirements:**
- All planning documents MUST be created BEFORE code implementation
- ESP Core 0 must remain isolated (control loop only)
- CAN RX/TX must run on ESP Core 1 exclusively  
- No blocking calls on ESP
- Follow AGENTS.md hard rules strictly

**Human Review Required:**
1. Confirm PONG command ID allocation (recommended: 0x001 under STATUS class → ID=0x111)
2. Confirm ESP32 GPIO pin assignments for CAN TX/RX
3. Confirm SocketCAN interface name on Pi (can0 vs can1)
4. Approve acceptable RTT threshold (currently 100ms)

**Implementation Sequence:**
See Section 9 of `roadmap_can_ping_pong.md` for detailed 17-step sequence.

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
