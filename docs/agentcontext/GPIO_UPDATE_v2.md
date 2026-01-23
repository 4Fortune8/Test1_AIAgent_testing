# CAN Bus Configuration Update - GPIO Pin Change
**Date:** 2026-01-22  
**Version:** v2.0  
**Status:** Configuration Updated

---

## Summary of Changes

The GPIO pin configuration for ESP32 CAN communication has been updated based on the user-provided YAML specification:

### Previous Configuration (v1.0)
- **CAN TX:** GPIO21 (Pin 33)
- **CAN RX:** GPIO22 (Pin 36)

### New Configuration (v2.0) ✅
- **CAN TX:** GPIO5 (Pin 29)
- **CAN RX:** GPIO4 (Pin 26)

### Rationale for GPIO5/4
- **Safe boot behavior:** No conflicts with boot mode pins (GPIO0, GPIO2, GPIO12)
- **Not SPI flash:** Avoids GPIO6-11 which would brick the device
- **Not UART0:** Avoids GPIO1/3 used for programming/debug
- **User specified:** Marked as "safe default, overrides GPIO21 from doc" in YAML

---

## Raspberry Pi Pin Corrections

The Raspberry Pi SPI pin assignments have also been corrected:

### Previous (Incorrect)
- MOSI: GPIO9 (Pin 21)
- MISO: GPIO11 (Pin 23)
- SCLK: GPIO10 (Pin 23)
- CS: GPIO7 (Pin 26)
- INT: GPIO8 (Pin 24)

### Current (Corrected) ✅
- **MOSI: GPIO10 (Pin 19)**
- **MISO: GPIO9 (Pin 21)**
- **SCLK: GPIO11 (Pin 23)**
- **CS: GPIO8 (Pin 24)**
- **INT: GPIO25 (Pin 22)**

---

## Updated Documents

The following documentation has been updated with the new configuration:

### Primary Documentation
1. **`docs/agentcontext/roadmap_can_ping_pong.md`** ✅
   - Section 0: Hardware State (GPIO5/4, complete Pi SPI wiring)
   - Section 1.1: ESP planning requirements (GPIO5/4 rationale)
   - Section 1.2: Pi planning requirements (corrected SPI pins)
   - Section 3.1: TWAI initialization (GPIO_NUM_5/4 defines)
   - Section 4.1: SocketCAN config (interrupt=25 for GPIO25)
   - Section 4.1: Failure modes (GPIO25 interrupt troubleshooting)
   - Section 13: Human review points (updated wiring action items)

2. **`docs/agentcontext/hardware_wiring_guide_v2.md`** ✅ (NEW FILE)
   - Complete wiring guide with GPIO5/4 configuration
   - Corrected Pi SPI pin table
   - Updated system diagram
   - Updated `/boot/config.txt` instructions (interrupt=25)
   - Updated troubleshooting section
   - Updated verification checklist

### Pending Updates
The following files still contain old GPIO21/22 references and need manual review:

3. **`docs/agentcontext/checklist_can_ping_pong.md`** ⚠️
   - Line 28: Still shows GPIO21/22
   - Needs: Update to GPIO5/4 with timestamp

4. **`docs/agentcontext/PLANNING_SUMMARY.md`** ⚠️
   - Hardware configuration section still shows GPIO21/22
   - Needs: Update to GPIO5/4, corrected Pi SPI pins

5. **`docs/agentcontext/QUICKSTART.txt`** ⚠️
   - Section 3 still shows GPIO21/22
   - Needs: Update to GPIO5/4

6. **`docs/agentcontext/hardware_wiring_guide.md`** (ORIGINAL)
   - Contains old GPIO21/22 configuration
   - **Recommendation:** Rename to `hardware_wiring_guide_v1_deprecated.md` for history
   - Use `hardware_wiring_guide_v2.md` as primary reference

---

## `/boot/config.txt` Configuration Change

The MCP2515 overlay configuration has been updated to reflect GPIO25 for interrupt:

### Previous
```
dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=8
```

### Current ✅
```
dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25,spimaxfrequency=1000000
```

**Parameters:**
- `oscillator=8000000` → Must match MCP2515 crystal (8 or 16 MHz)
- `interrupt=25` → GPIO25 (Pin 22) connected to MCP2515 INT
- `spimaxfrequency=1000000` → Conservative 1 MHz SPI (can tune 1-10 MHz)

---

## Code Changes Required

When implementing the ESP32 firmware, use these pin definitions:

```cpp
// ESP32 CAN GPIO configuration
#define CAN_TX_GPIO 5   // GPIO5, Pin 29 on ESP32-WROOM-32D
#define CAN_RX_GPIO 4   // GPIO4, Pin 26 on ESP32-WROOM-32D

// TWAI driver configuration
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
    GPIO_NUM_5,  // TX
    GPIO_NUM_4,  // RX
    TWAI_MODE_NORMAL
);
```

---

## Physical Wiring Checklist

### ESP32 to SN65HVD230
- [ ] GPIO5 (Pin 29) → SN65HVD230 TX
- [ ] GPIO4 (Pin 26) → SN65HVD230 RX
- [ ] 3.3V (Pin 2) → SN65HVD230 VCC
- [ ] GND → SN65HVD230 GND
- [ ] SN65HVD230 RS → GND (high-speed mode)

### Raspberry Pi to MCP2515
- [ ] GPIO10 (Pin 19) → MCP2515 SI (MOSI)
- [ ] GPIO9 (Pin 21) → MCP2515 SO (MISO)
- [ ] GPIO11 (Pin 23) → MCP2515 SCK
- [ ] GPIO8 (Pin 24) → MCP2515 CS
- [ ] GPIO25 (Pin 22) → MCP2515 INT
- [ ] 3.3V (Pin 1) → MCP2515 VCC
- [ ] GND → MCP2515 GND

### CAN Bus
- [ ] SN65HVD230 CANH → MCP2515 CANH (twisted pair)
- [ ] SN65HVD230 CANL → MCP2515 CANL (twisted pair)
- [ ] 120Ω resistor at ESP32/SN65HVD230 end
- [ ] 120Ω resistor at Pi/MCP2515 end
- [ ] Verify ~60Ω total resistance between CAN_H and CAN_L
- [ ] Shared ground between ESP32 and Pi

---

## User's YAML Specification (Reference)

The user provided the following configuration:

```yaml
nodes:
  - name: ESP32_WROOM_32D
    twai_pins:
      TX: GPIO5    # safe default, overrides GPIO21 from doc
      RX: GPIO4
    RS_pin: GND    # high-speed mode, max bitrate
    
  - name: Raspberry_Pi_3B+
    can_transceiver: MCP2515
    spi_pins:
      MOSI: GPIO10  # Pin 19
      MISO: GPIO9   # Pin 21
      SCLK: GPIO11  # Pin 23
      CS: GPIO8     # Pin 24
      INT: GPIO25   # Pin 22 or user-defined
```

This YAML specification is now reflected in all updated documentation.

---

## Next Steps

1. **Manual Review Required:**
   - Review `checklist_can_ping_pong.md` and update GPIO references
   - Review `PLANNING_SUMMARY.md` and update hardware section
   - Review `QUICKSTART.txt` and update section 3

2. **Physical Wiring:**
   - Follow `hardware_wiring_guide_v2.md` for step-by-step instructions
   - Verify all connections before powering on
   - Install termination resistors at both ends

3. **Raspberry Pi Configuration:**
   - Edit `/boot/config.txt` with new overlay parameters
   - Reboot and verify MCP2515 initialization
   - Bring up `can0` interface

4. **Implementation Planning:**
   - Create ESP planning document with GPIO5/4 configuration
   - Create Pi planning document with corrected SPI pins
   - Proceed with TWAI driver implementation

---

**Approved By:** User (YAML specification provided 2026-01-22)  
**Implementation Status:** Documentation updated, physical wiring pending
