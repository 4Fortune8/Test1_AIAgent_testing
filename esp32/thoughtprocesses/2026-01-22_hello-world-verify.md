# ESP32 Hello World - Connection Verification

**Date:** 2026-01-22  
**Version:** V1.03.00 (Step: Basic ESP32 setup before CAN)  
**User Request:** "lets begin ESP32 firmware development. start with a basic hello world check connections"

---

## Problem Statement

Before implementing CAN TWAI driver, verify:
1. ESP32 can be programmed via PlatformIO
2. Serial monitor communication works
3. GPIO5/4 pins are accessible (won't brick device)
4. Basic Arduino framework operational

---

## Constraints

- **Hardware:** ESP32-WROOM-32D on UPeasy devkit
- **Build System:** PlatformIO with Arduino framework
- **GPIO Pins:** GPIO5 (CAN TX), GPIO4 (CAN RX) - must not interfere with boot
- **Hard Rules:** No blocking delays, verify non-blocking patterns work
- **Serial:** 115200 baud for debug output

---

## Approach

### Step 1: Create Basic Hello World
- Print startup message
- Blink onboard LED (if available, typically GPIO2)
- Report GPIO5/4 pin states
- Loop with 1-second interval (non-blocking with millis())

### Step 2: Verify GPIO Safety
- Read GPIO5/4 states (input mode)
- Confirm ESP32 boots without issues
- Verify no boot mode conflicts

### Step 3: Test Serial Communication
- Print continuous heartbeat messages
- Verify Serial monitor receives data at 115200 baud

---

## Files to Modify

1. **CREATE:** `esp/src/main.cpp` - Main Arduino sketch
   - Setup: Serial init, GPIO configuration, startup banner
   - Loop: Heartbeat with millis(), GPIO status reporting

---

## Implementation Plan

```cpp
// Basic structure:
void setup() {
  Serial.begin(115200);
  delay(100); // Allow serial to initialize
  
  // Print startup banner
  Serial.println("\n=== ESP32 Hello World ===");
  Serial.println("Board: ESP32-WROOM-32D (UPeasy)");
  Serial.println("CAN Pins: GPIO5 (TX), GPIO4 (RX)");
  
  // Configure GPIO as inputs (safe default)
  pinMode(5, INPUT);
  pinMode(4, INPUT);
  
  // Optional: Onboard LED
  pinMode(2, OUTPUT);
}

void loop() {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  
  // Non-blocking 1-second heartbeat
  if (now - lastPrint >= 1000) {
    lastPrint = now;
    
    Serial.print("Heartbeat: ");
    Serial.print(now / 1000);
    Serial.print("s | GPIO5: ");
    Serial.print(digitalRead(5));
    Serial.print(" | GPIO4: ");
    Serial.println(digitalRead(4));
    
    // Toggle LED
    digitalWrite(2, !digitalRead(2));
  }
}
```

---

## Risks

1. **GPIO5/4 might affect boot** - Mitigated: Using INPUT mode only, no OUTPUT
2. **Serial buffer overflow** - Mitigated: 1-second intervals, short messages
3. **PlatformIO not configured** - Mitigated: platformio.ini already exists with correct settings

---

## Testing Strategy

### Success Criteria:
- [ ] ESP32 boots without errors
- [ ] Serial monitor shows startup banner at 115200 baud
- [ ] Heartbeat messages appear every 1 second
- [ ] GPIO5/4 states are readable (likely HIGH due to pull-ups)
- [ ] LED blinks (if GPIO2 is onboard LED)
- [ ] No boot loops or crashes

### Test Commands:
```bash
# Build firmware
cd esp
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

---

## Exit Criteria

✅ ESP32 successfully programmed  
✅ Serial communication verified  
✅ GPIO5/4 accessible without boot issues  
✅ Non-blocking loop pattern confirmed  
✅ Ready for TWAI driver implementation (next step: V1.04.00)

---

## Next Steps (After Success)

1. Create `2026-01-22_can-bringup-esp.md` for TWAI driver planning
2. Implement TWAI initialization (Phase 3.1 of roadmap)
3. Test CAN loopback mode before connecting to Pi

---

**Planning Document Complete - Ready for Implementation**
