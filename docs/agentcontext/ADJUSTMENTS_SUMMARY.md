# Roadmap Adjustments Complete - Summary

**Date:** 2026-01-22  
**Agent:** GitHub Copilot (Planning)  
**Status:** ✅ All Requested Adjustments Implemented

---

## Changes Made

### 1. ✅ CAN ID Math: Decimal vs Hex Clarity

**Files Updated:**
- `protocol/can_id_map.md` - Complete rewrite with comprehensive tables

**What Changed:**
- Added complete binary/hex/decimal reference table
- PING: Binary `001 0001 0001` | Hex `0x091` | Decimal `145`
- PONG: Binary `010 0001 0001` | Hex `0x111` | Decimal `273`
- Structured tables for Message Classes, Node IDs, Command IDs, Status IDs
- Added note: "Use hex for clarity in code, decimal for logging only"

**Benefit:**
- Eliminates silent off-by-one errors in implementation
- Single source of truth for all ID math
- Clear distinction between binary (protocol), hex (code), decimal (debug)

---

### 2. ✅ PONG Payload Locked to 4 Bytes

**Files Updated:**
- `roadmap_can_ping_pong.md` - Section 2.2 (PONG specification)
- `protocol_amendment_pong.md` - Status changed to APPROVED

**What Changed:**
- PONG payload: **REQUIRED** 4 bytes (was optional)
- Payload: `uint32_t` monotonic sequence counter (little-endian)
- Starts at 0 on ESP boot, increments per PONG, wraps at UINT32_MAX

**Benefits:**
- ✅ Detect duplicate frames (same counter)
- ✅ Detect ESP reboot (counter reset)
- ✅ Detect lost frames (counter gaps)
- ✅ Rich diagnostics at 32 bps @ 1 Hz (negligible cost)
- ✅ Future-proof without protocol change

**Implementation Details:**
- ESP: `data[0-3] = sequence_counter` (little-endian), then `sequence_counter++`
- Pi: Validate DLC==4, parse counter, check for duplicates/gaps/resets

---

### 3. ✅ CAN Filters Explicitly Documented

**Files Updated:**
- `roadmap_can_ping_pong.md` - Section 3.1 (TWAI config)

**What Changed:**
- **Initial:** Accept all messages (filter disabled for debugging)
- **Production (documented for future):**
  - Filter for Class=COMMAND (001) OR Class=ERROR (100)
  - Node=ESP32-1 (0001)
  - Acceptance mask: `0x1F0` (bits 8,7,6,5,4 matter)
  - Acceptance code: `0x090` (matches `0x09X` = COMMAND to ESP32-1)
  - Note about dual filter mode for multiple classes

**Benefit:**
- Future agents know exactly how to enable filtering
- Clear reasoning for disabled state during bring-up
- Production configuration pre-planned

---

### 4. ✅ Single-Node Assertion Added

**Files Updated:**
- `roadmap_can_ping_pong.md` - Sections 2.1, 2.2, 3.3, 4.4, 5.2, 7.6
- `checklist_can_ping_pong.md` - Test 8 added

**What Changed:**

**ESP Side (Section 3.3):**
- Added node ID filtering in PING handler
- ESP ignores PING if Node ID != 0x1 (not addressed to this ESP)
- Silently ignore (no log spam in multi-node networks)

**Pi Side (Section 2.2, 4.4):**
- Validate Node ID in every PONG: `(msg_id >> 4) & 0x0F == 0x1`
- Track if multiple PONGs received per PING
- Log ERROR if multi-PONG detected (should never happen with single ESP)

**Testing (Section 5.2):**
- New Test 8: Multi-Node Detection
- Verifies single-node assertion works
- Pi correctly identifies multi-node responses

**Benefits:**
- ✅ Prevents phantom success when scaling to multi-ESP later
- ✅ Catches wiring issues (cable loops, reflections)
- ✅ Catches accidental multi-node setups during testing
- ✅ ESP ignores traffic not meant for it (future multi-node readiness)

---

### 5. ✅ CAN Invariants Formalized

**New File Created:**
- `protocol/can_invariants.md` - 15 binding invariants

**Contents:**
```
1-4:   ESP Real-Time Invariants (non-blocking, control loop independence, memory safety, determinism)
5-7:   Pi Non-Real-Time Invariants (assume ESP disappearance, no timing assumptions, graceful degradation)
8-10:  Protocol Invariants (idempotency, node ID validation, payload consistency)
11-13: Communication Invariants (unidirectional intent, no implicit state, fail-safe defaults)
14-15: Testing Invariants (isolated testing, deterministic logging)
```

**Key Rules Codified:**
- ESP must NEVER block on CAN TX/RX
- Control loop timing must NEVER depend on CAN
- Pi must assume ESP can disappear at any time
- CAN messages must be idempotent (state-setting, not toggling)
- Node ID validation required on both sides
- All failures default to "do nothing, log warning"

**Integration:**
- Referenced in roadmap Section 0 (prerequisites)
- Referenced in roadmap Section 10 (compliance checklist)
- Required reading in all planning documents (Section 1)

**Benefit:**
- ✅ Prevents timing-dependent bugs (hardest to debug)
- ✅ Prevents cascade failures across nodes
- ✅ Ensures safe operation under all fault conditions
- ✅ Documents "why" for future maintainers

---

## Summary of Protocol Changes

### Before Adjustments:
- PONG ID: Not allocated
- PONG payload: Optional, unspecified
- CAN ID math: Inconsistent format (binary or hex)
- Node filtering: Not specified
- CAN filters: Mentioned, not documented
- Invariants: Implied, not formalized

### After Adjustments:
- ✅ PONG ID: 0x111 (STATUS class, fully documented with binary/hex/decimal)
- ✅ PONG payload: 4 bytes, uint32 counter, little-endian (REQUIRED)
- ✅ CAN ID math: Complete reference table, all three formats
- ✅ Node filtering: ESP ignores wrong node, Pi validates source
- ✅ CAN filters: Production config documented, disabled during bring-up
- ✅ Invariants: 15 binding rules in standalone document

---

## Files Modified/Created

### Modified:
1. `protocol/can_id_map.md` - Complete rewrite with tables
2. `docs/agentcontext/roadmap_can_ping_pong.md` - 14 sections updated
3. `docs/agentcontext/protocol_amendment_pong.md` - Status → APPROVED
4. `docs/agentcontext/checklist_can_ping_pong.md` - 4 sections updated
5. `docs/agentcontext/PLANNING_SUMMARY.md` - 3 sections updated
6. `docs/agentcontext/QUICKSTART.txt` - Header and tables updated

### Created:
7. `protocol/can_invariants.md` - NEW (15 invariants, ~200 lines)

---

## Remaining Human Decisions

**Protocol decisions:** ✅ COMPLETE (PONG ID, payload format)

**Hardware configuration:** ⏳ PENDING
- ESP32 CAN TX GPIO pin number
- ESP32 CAN RX GPIO pin number
- Raspberry Pi SocketCAN interface name

**RTT threshold:** ⏳ PENDING or ✓ APPROVE DEFAULT
- Default: <50ms target, <100ms max
- Confirm or adjust based on system requirements

---

## Impact on Implementation

**No Breaking Changes:**
- All adjustments are additive or clarifying
- No changes to existing code (none written yet)
- Planning discipline maintained

**Implementation Agents Should:**
1. Read `protocol/can_invariants.md` FIRST
2. Use `protocol/can_id_map.md` as canonical reference for ALL IDs
3. Implement 4-byte PONG payload per specification
4. Include node ID filtering in ESP PING handler
5. Include single-node assertion in Pi PONG reception
6. Document future CAN filter config per roadmap Section 3.1
7. Reference invariants in planning documents

**Testing Agents Should:**
- Add Test 8 (multi-node detection)
- Add Test 9 (sequence counter validation)
- Verify counter starts at 0, increments, detects reboots
- Verify node ID filtering works (send PING with wrong node)

---

## Compliance

✅ Followed AGENTS.md planning discipline  
✅ No code written  
✅ All changes documented  
✅ Human review points identified  
✅ Backward compatibility maintained (no existing code)  
✅ Safety and robustness enhanced  

---

## Next Steps

**For Human:**
1. Review this summary
2. Confirm hardware GPIO assignments
3. Approve to proceed with implementation

**For Implementation Agents:**
1. Wait for hardware config approval
2. Create 3 planning documents (ESP, Pi, Protocol)
3. Reference new invariants and ID tables in all planning
4. Follow roadmap Section 9 sequence exactly

---

**All requested adjustments implemented successfully.**  
**Roadmap is now more robust, precise, and future-proof.**
