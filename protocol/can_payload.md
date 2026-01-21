# CAN Motion Payload Formats

All values are little-endian.
All floats are IEEE-754 32-bit.

---

## COMMAND: SET_TARGET_POSITION

- CAN Class: COMMAND
- Payload length: 4 bytes

| Byte | Meaning |
|----|--------|
| 0–3 | float32 target_position_mm |

Unused bytes must be zero.

---

## COMMAND: SET_TARGET_VELOCITY

- Payload length: 4 bytes

| Byte | Meaning |
|----|--------|
| 0–3 | float32 target_velocity_mm_s |

---

## COMMAND: SET_ACCEL_LIMIT

- Payload length: 4 bytes

| Byte | Meaning |
|----|--------|
| 0–3 | float32 accel_limit_mm_s2 |

---

## COMMAND: ENABLE_MOTOR

- Payload length: 0 bytes

---

## COMMAND: DISABLE_MOTOR

- Payload length: 0 bytes

---

## STATUS: POSITION_REPORT

- Payload length: 4 bytes

| Byte | Meaning |
|----|--------|
| 0–3 | float32 current_position_mm |

---

## STATUS: VELOCITY_REPORT

- Payload length: 4 bytes

| Byte | Meaning |
|----|--------|
| 0–3 | float32 current_velocity_mm_s |

---

## ERROR: FAULT_CODE

- Payload length: 1 byte

| Byte | Meaning |
|----|--------|
| 0 | fault_code enum |
