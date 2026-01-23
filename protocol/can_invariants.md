# CAN Invariants

**Purpose:** Immutable rules that govern all CAN communication in this system.

**Status:** BINDING - Violations are critical bugs

---

## Core Invariants

### ESP Real-Time Invariants

1. **Non-Blocking CAN Operations**
   - ESP must NEVER block on CAN TX/RX operations
   - All TWAI calls must have timeouts
   - If TX queue full: drop message, log, continue
   - If RX queue empty: continue without waiting

2. **Control Loop Independence**
   - Control loop timing must NEVER depend on CAN communication
   - Control loop runs on Core 0, isolated from CAN (Core 1)
   - CAN task failure must not crash control loop
   - Control loop must maintain safe state if CAN dies

3. **Memory Safety**
   - No dynamic allocation in CAN hot paths
   - Message buffers pre-allocated at init
   - No heap operations in interrupt context
   - Stack sizes verified and documented

4. **Deterministic Response**
   - PING → PONG response time bounded (target <10ms)
   - No unbounded loops in CAN handlers
   - Message processing time profiled and documented

---

### Raspberry Pi Non-Real-Time Invariants

5. **Assume ESP Disappearance**
   - Pi must assume ESP can disappear at any time
   - All commands must have timeout (default 100ms)
   - Timeout is not an error, it's expected behavior
   - Pi must log and continue, never hang waiting

6. **No Timing Assumptions**
   - Pi cannot assume deterministic CAN latency
   - Pi cannot assume message delivery order
   - Pi must handle out-of-order responses
   - Pi must handle duplicate messages

7. **Graceful Degradation**
   - CAN interface down → Pi logs, attempts recovery
   - ESP not responding → Pi logs, continues operation
   - Malformed messages → Pi logs, ignores, continues

---

### Protocol Invariants

8. **Message Idempotency**
   - Repeated delivery of same message must be safe
   - Commands must be state-setting, not state-toggling
   - Status messages must be absolute, not deltas
   - Example: SET_POSITION(100mm) not MOVE_RELATIVE(+10mm)

9. **Node ID Validation**
   - ESP must ignore messages not addressed to its node ID
   - Pi must validate node ID in all responses
   - Multi-node operation: only addressed node responds
   - Broadcast messages (Node=0x0) explicitly documented when added

10. **Payload Consistency**
    - All multi-byte values use little-endian
    - Unused payload bytes must be zero
    - DLC must match documented payload length exactly
    - Parsers must validate DLC before accessing payload

---

### Communication Invariants

11. **Unidirectional Intent**
    - Pi sends commands (intent)
    - ESP sends status (state)
    - ESP never sends commands to Pi
    - Pi never assumes ESP will obey (validate via status)

12. **No Implicit State**
    - Every message is self-contained
    - No hidden state machines in protocol
    - No "session" or "connection" concept
    - Either node can restart without handshake

13. **Fail-Safe Defaults**
    - Unknown message class → ignore
    - Unknown command ID → ignore (do not error)
    - Invalid payload → ignore message entirely
    - When in doubt: do nothing, log warning

---

### Testing Invariants

14. **Isolated Testing**
    - Pi code must run without hardware (mock mode)
    - ESP code must be testable without Pi (use cansend)
    - Tests must not require both nodes simultaneously
    - Failure injection must be possible at any layer

15. **Deterministic Logging**
    - All CAN operations must be logged (timestamped)
    - Logs must include: ID, DLC, payload, direction
    - Errors logged with context (state, counters)
    - Logs parsable by automated tools

---

## Enforcement

- Code reviews must verify invariants
- Tests must exercise invariant violations
- Static analysis checks where possible
- Documentation must reference relevant invariants

---

## Rationale

These invariants exist because:
- **Real-time systems require determinism**
- **Distributed systems require fault tolerance**
- **CAN networks are unreliable by nature**
- **Humans debug what code documents**

Violating these invariants leads to:
- Timing-dependent bugs (hardest to debug)
- Cascade failures across nodes
- Non-reproducible behavior
- Unsafe motor operation

---

## Version History

- 2026-01-22: Initial invariants established during CAN PING/PONG planning
