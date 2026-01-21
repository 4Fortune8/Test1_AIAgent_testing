## Raspberry Pi (Non-Real-Time)

Responsibilities:
- Motion planning
- Trajectory generation
- Coordinate transforms
- UI / visualization
- Logging and debugging

Forbidden:
- Encoder counting
- PID loops
- Motor timing
- Safety-critical real-time logic

## ESP32 (Real-Time)

Responsibilities:
- Encoder decoding (PCNT / ISR)
- Velocity & position PID
- Acceleration limiting
- Safety handling
- PWM + direction output

Timing requirements:
- Control loop: 1–5 kHz
- Encoder ISR latency < 5 µs

ESP firmware must remain safe if Pi disconnects.
