# CAN PING/PONG Planning Summary

**Date:** 2026-01-22  
**Planning Agent:** GitHub Copilot  
**Status:** Planning Complete - Awaiting Human Review

---

## What Was Created

This planning session produced a comprehensive roadmap for establishing minimal CAN communication between the Raspberry Pi and ESP32.

### Documents Created

1. **`docs/agentcontext/roadmap_can_ping_pong.md`** (Main Deliverable)
   - 13 sections, ~650 lines
   - Complete implementation roadmap from hardware check to 24-hour stability test
   - Includes failure modes, testing strategy, exit criteria
   - Designed to be followed step-by-step by implementation agents

2. **`docs/agentcontext/checklist_can_ping_pong.md`**
   - Implementation tracking checklist
   - All phases, tests, and exit criteria in checkbox format
   - Quick reference for agents and humans

3. **`docs/agentcontext/protocol_amendment_pong.md`** ✅ APPROVED
   - ~~Proposal~~ Decision document for PONG command ID
   - ~~Two options presented~~ Approved: STATUS class, 4-byte payload
   - Complete specification and rationale

4. **`docs/agentcontext/README.md`** (Updated)
   - Added active roadmap information
   - Human review requirements listed

---

## Key Planning Decisions

### Architecture Preserved
✅ ESP Core 0 remains isolated (control loop only)  
✅ CAN RX/TX limited to ESP Core 1  
✅ No blocking calls introduced  
✅ Pi remains non-real-time  
✅ All existing protocol contracts honored

### Scope Strictly Limited
✅ PING/PONG communication only  
✅ No motor control  
✅ No encoder integration  
✅ No Wi-Fi or fallback transports  
✅ Single ESP node (ESP32-1)

### Safety & Robustness Emphasized
✅ Bus-off recovery mechanisms planned  
✅ Timeout handling on both sides  
✅ Graceful error handling for cable disconnect  
✅ 24-hour stability test required before exit

### Protocol Enhancements
✅ **CAN ID Math:** Complete binary/hex/decimal table in `can_id_map.md`  
✅ **PONG Payload:** 4-byte sequence counter (duplicate/reboot detection)  
✅ **CAN Filters:** Documented for future use (disabled during bring-up)  
✅ **Single-Node Assertion:** ESP filters by node ID, Pi detects multi-PONG  
✅ **Invariants:** 15 binding rules in `protocol/can_invariants.md`

---

## What Needs Human Review

### ~~Critical Decision: PONG Command ID~~ ✅ APPROVED

**Decision Made:** STATUS class, ID=0x001 → CAN ID=0x111, DLC=4 (sequence counter)

**Implementation Details:**
- 4-byte uint32 sequence counter (little-endian)
- Starts at 0 on boot, increments per PONG
- Enables duplicate detection, reboot detection, diagnostics
- See `protocol_amendment_pong.md` for complete specification

---

### ~~Hardware Configuration Confirmation~~ ✅ COMPLETE

**Approved Configuration:**
1. **ESP32 Hardware:**
   - Board: ESP32-WROOM-32D on UPeasy devkit
   - Transceiver: SN65HVD230 (3.3V logic)
   - CAN TX: GPIO21 (Pin 33)
   - CAN RX: GPIO22 (Pin 36)

2. **Raspberry Pi Hardware:**
   - Board: Raspberry Pi 3B+
   - Transceiver: MCP2515 CAN Bus Module (SPI)
   - Interface name: `can0`

3. **RTT Threshold:**
   - Target: <50ms average
   - Maximum: 100ms
   - Status: ✅ APPROVED

**Pre-Implementation Actions Required:**
- ⚠️ Wire SN65HVD230 to ESP32 GPIO21/22 (see roadmap Section 13)
- ⚠️ Install 120Ω termination resistors at both ends
- ⚠️ Configure Pi `/boot/config.txt` with MCP2515 overlay

---

## Implementation Readiness

### Before ANY Code Is Written

**MUST CREATE (per AGENTS.md):**
- [ ] `esp/thoughtprocesses/2026-01-22_can-bringup-esp.md`
- [ ] `pi/thoughtprocesses/2026-01-22_can-bringup-pi.md`
- [ ] `protocol/thoughtprocesses/2026-01-22_ping-pong-spec.md`

These planning documents must include:
- User request verbatim
- Problem statement and constraints
- Step-by-step approach
- Files to modify and why
- Risks and mitigation
- Testing strategy

### Implementation Sequence (17 Steps)

See Section 9 of `roadmap_can_ping_pong.md` for full sequence.

**Phase Order:**
1. Planning docs → 2. Protocol amendment → 3. ESP TWAI init → 4. Pi SocketCAN → 5. ESP RX task → 6. PING transmission → 7. PONG response → 8. Error handling → 9. Testing → 10. Validation

**Estimated Effort:**
- Planning docs: 1-2 hours
- ESP implementation: 4-6 hours
- Pi implementation: 2-3 hours
- Testing & validation: 24+ hours (includes long-duration test)

---

## Exit Criteria Summary

**Do NOT proceed to motor commands until:**
- ✅ >99% PING/PONG success over 24 hours
- ✅ RTT <50ms average, <100ms p99
- ✅ No crashes, panics, or memory leaks
- ✅ ESP recovers from bus-off automatically
- ✅ Pi recovers from interface restart
- ✅ Cable disconnect/reconnect handled
- ✅ Core isolation verified (Core 0 uninterrupted)
- ✅ All planning docs complete with version tags
- ✅ All tests passed and logged

---

## Risk Assessment

**High Risk (Mitigated):**
- ESP control loop blocked → Core affinity + audit
- Bus-off unrecoverable → Auto-recovery logic

**Medium Risk:**
- Timing jitter >100ms → Profiling available

**Low Risk:**
- Library compatibility → Version pinning

---

## Compliance with AGENTS.md

✅ **No code written** (planning only)  
✅ **No architectural refactors** proposed  
✅ **Modifying existing files** preferred  
✅ **Planning documents** required before implementation  
✅ **Version tagging** system established  
✅ **Hard rules** strictly followed:
- No blocking delays on ESP
- No real-time logic on Pi
- No protocol format changes (only addition)
- No hardware code refactoring for style

✅ **docs/roles.md** respected (ESP real-time, Pi planning)  
✅ **protocol/motion_contract.md** honored (not applicable yet, but aware)

---

## Next Steps for Human

### Immediate Actions
1. **Review** `roadmap_can_ping_pong.md` in full
2. **Decide** on PONG command ID (see `protocol_amendment_pong.md`)
3. **Confirm** ESP GPIO pins and Pi interface name
4. **Approve** roadmap for implementation

### If Approved
5. Update `protocol/can_id_map.md` with PONG ID
6. Update `protocol/can_payload.md` with PONG spec
7. Provide go-ahead to implementation agents

### If Changes Needed
5. Provide feedback on roadmap sections to adjust
6. Request revised planning if scope changes

---

## Next Steps for Implementation Agent

**AFTER human approval:**
1. Create three planning documents (ESP, Pi, Protocol)
2. Wait for human review of planning docs
3. Follow roadmap Section 9 implementation sequence exactly
4. Add version tags to all code changes
5. Run tests after each phase
6. Update `version_log.md` on completion

**DO NOT:**
- Skip planning document creation
- Proceed without human approval
- Deviate from roadmap sequence
- Invent new protocol fields
- Refactor working code for style

---

## Questions?

If anything in the roadmap is unclear:
- Reference section numbers for specifics
- Consult AGENTS.md for architectural rules
- Consult docs/roles.md for responsibility boundaries
- Ask human before making assumptions

---

## Success Definition

**Planning is successful if:**
- Implementation agent can follow roadmap without ambiguity
- All failure modes are identified and handled
- Human can review and approve with confidence
- No architectural violations occur during implementation
- Testing strategy validates all requirements

**Implementation will be successful if:**
- All exit criteria met (see roadmap Section 7)
- 24-hour stability test passes
- RTT consistently <50ms
- No crashes or memory leaks
- Ready for next phase (motor commands)

---

**End of Planning Summary**

This planning session followed AGENTS.md strictly.  
No code was written.  
No protocol changes were made.  
All decisions are documented and reversible.  

Ready for human review.
