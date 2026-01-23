# Protocol Amendment: PONG Command

**Date:** 2026-01-22  
**Status:** ✅ APPROVED - Implementation Ready  
**Related Roadmap:** `docs/agentcontext/roadmap_can_ping_pong.md`

---

## Decision Summary

**APPROVED CONFIGURATION:**
- **Command ID:** STATUS class, ID=0x001 (CAN ID=0x111)
- **Payload:** 4 bytes, uint32 sequence counter (little-endian)
- **Rationale:** Enables duplicate detection, reboot detection, and diagnostics

---

## Implementation Details

### PONG Message Specification

**CAN ID:**
```
Message Class: 010 (STATUS)
Node ID:       0001 (ESP32-1)
Status ID:     0001 (PONG)

Binary:  010 0001 0001
Hex:     0x111
Decimal: 273
```

**Payload Format:**
```
DLC: 4 bytes

Byte 0: Sequence counter LSB (bits 0-7)
Byte 1: Sequence counter (bits 8-15)
Byte 2: Sequence counter (bits 16-23)
Byte 3: Sequence counter MSB (bits 24-31)

Encoding: Little-endian, unsigned 32-bit
```

**Sequence Counter Behavior:**
- Initializes to 0 on ESP boot
- Increments after each PONG transmission (post-increment)
- Wraps at UINT32_MAX (0xFFFFFFFF → 0x00000000)
- Allows detection of:
  - Duplicate frames (same counter value)
  - Lost frames (counter gaps)
  - ESP reboot (counter reset to low values)

---

## Protocol Files Updated

### `protocol/can_id_map.md`
Added complete reference table with binary, hex, and decimal values:
- PING: 0x091 (145 decimal)
- PONG: 0x111 (273 decimal)

### `protocol/can_invariants.md`
Created new file with 15 binding invariants covering:
- ESP real-time requirements
- Pi fault tolerance
- Protocol idempotency
- Node ID validation
- Testing requirements

---

## Benefits of 4-Byte Payload

1. **Duplicate Detection:**
   - Same sequence = duplicate/reflected message
   - Pi can filter duplicates in noisy environments

2. **Reboot Detection:**
   - Counter reset (new << previous) = ESP restarted
   - Pi can log restarts without separate messages

3. **Diagnostics:**
   - Counter gaps = missed PONGs (ESP or Pi issue)
   - Counter rate = actual response rate vs expected

4. **Future-Proof:**
   - No protocol change needed for advanced diagnostics
   - Minimal overhead: 4 bytes @ 1 Hz = 32 bps

5. **Zero Cost Alternative:**
   - If hardware constraints arise, can reduce to DLC=0
   - But recommended to keep for production systems

---

## Comparison with 0-Byte Alternative

| Feature | 0-Byte (DLC=0) | 4-Byte (DLC=4) |
|---------|----------------|----------------|
| Bus overhead | 0 bytes | 4 bytes |
| Duplicate detection | No | Yes |
| Reboot detection | No | Yes |
| Lost frame detection | No | Yes |
| Diagnostics | Minimal | Rich |
| Bandwidth @ 1 Hz | 0 bps | 32 bps |
| Bandwidth @ 10 Hz | 0 bps | 320 bps |

**Decision:** 4-byte payload provides significant diagnostic value at negligible cost.

---

## Changes Made to Roadmap

1. ✅ Section 0: Added PONG ID to prerequisites
2. ✅ Section 1.3: Updated protocol planning requirements with sequence counter
3. ✅ Section 2.2: Complete PONG specification with counter behavior
4. ✅ Section 3.4: ESP TX implementation includes counter increment
5. ✅ Section 4.4: Pi RX implementation includes sequence validation
6. ✅ Section 5.2: Added Tests 8-9 for sequence counter and multi-node detection
7. ✅ Section 7.6: Added counter verification to exit criteria
8. ✅ Section 9: Updated implementation sequence
9. ✅ Section 10: Added invariants and ID math compliance
10. ✅ Section 13: Marked PONG decisions as approved

---

## Validation Checklist

- [x] PONG ID allocated in `can_id_map.md`
- [x] Binary/Hex/Decimal table created
- [x] Payload format documented
- [x] Sequence counter behavior specified
- [x] Little-endian encoding confirmed
- [x] ESP implementation details in roadmap
- [x] Pi implementation details in roadmap
- [x] Test cases added for counter validation
- [x] Single-node assertion documented
- [x] CAN invariants created

---

## Next Steps

1. Implementation agents proceed with roadmap Section 9
2. Create planning documents referencing this amendment
3. Implement sequence counter per specification
4. Test counter behavior in Phase 3.4 / 4.4
5. Validate in long-duration test (should reach ~86,400 in 24 hours @ 1 Hz)

---

**This amendment is approved and ready for implementation.**
