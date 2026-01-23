Agents must not invent IDs.

## CAN ID Structure (11-bit)

```
Bits [10:8] = Message Class (3 bits)
Bits [7:4]  = Node ID (4 bits)
Bits [3:0]  = Command/Status ID (4 bits)
```

## Message Classes

| Binary | Hex | Decimal | Name       |
|--------|-----|---------|------------|
| 000    | 0x0 | 0       | System     |
| 001    | 0x1 | 1       | Command    |
| 010    | 0x2 | 2       | Status     |
| 011    | 0x3 | 3       | Telemetry  |
| 100    | 0x4 | 4       | Error      |

## Node IDs

| Binary | Hex | Decimal | Name     |
|--------|-----|---------|----------|
| 0001   | 0x1 | 1       | ESP32-1  |
| 0010   | 0x2 | 2       | ESP32-2  |

## Command IDs (Class 001)

| Binary | Hex | Decimal | Name     |
|--------|-----|---------|----------|
| 0001   | 0x1 | 1       | PING     |
| 0010   | 0x2 | 2       | LED_ON   |
| 0011   | 0x3 | 3       | LED_OFF  |

## Status IDs (Class 010)

| Binary | Hex | Decimal | Name     |
|--------|-----|---------|----------|
| 0001   | 0x1 | 1       | PONG     |

## Complete Message ID Reference

| Message | Class (Binary) | Node (Binary) | Cmd (Binary) | Full Binary     | Hex   | Decimal |
|---------|----------------|---------------|--------------|-----------------|-------|---------|
| PING    | 001            | 0001          | 0001         | 001 0001 0001   | 0x091 | 145     |
| PONG    | 010            | 0001          | 0001         | 010 0001 0001   | 0x111 | 273     |

**Note:** When constructing CAN IDs in code:
- Use hex for clarity: `0x091`, `0x111`
- Verify binary representation matches this table
- Decimal values provided for debugging/logging only
