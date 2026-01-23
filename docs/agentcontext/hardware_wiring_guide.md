# CAN Bus Hardware Wiring Guide
**Date:** 2026-01-22  
**System:** ESP32-WROOM-32D ↔ Raspberry Pi 3B+ CAN Communication  
**Status:** Ready for Physical Wiring

---

## Components Verified

### ESP32 Side
- **Board:** ESP32-WROOM-32D on UPeasy devkit
- **CAN Transceiver:** SN65HVD230 (3.3V logic)
- **Selected Pins:** GPIO21 (TX), GPIO22 (RX)

### Raspberry Pi Side
- **Board:** Raspberry Pi 3B+
- **CAN Transceiver:** MCP2515 CAN Bus Module (SPI interface)
- **Status:** ✅ Already wired to Pi SPI0

---

## ESP32 → SN65HVD230 Wiring

### Connection Table

| SN65HVD230 Pin | ESP32-WROOM-32D Pin | Pin Number | Notes |
|----------------|---------------------|------------|-------|
| VCC (3.3V)     | 3V3                 | Pin 2      | Power supply |
| GND            | GND                 | Pin 1, 15, or 38 | Ground |
| TX             | GPIO21              | Pin 33     | TWAI transmit |
| RX             | GPIO22              | Pin 36     | TWAI receive |
| RS             | GND                 | -          | High-speed mode (120kbps-1Mbps) |
| CANH           | CAN Bus H           | -          | To MCP2515 CANH |
| CANL           | CAN Bus L           | -          | To MCP2515 CANL |

### Wiring Diagram (Text)
```
ESP32-WROOM-32D          SN65HVD230           CAN Bus
┌──────────────┐         ┌──────────┐         ┌────────┐
│              │         │          │         │        │
│ 3V3 (Pin 2)  ├────────►│ VCC      │         │        │
│              │         │          │         │        │
│ GND (Pin 1)  ├────────►│ GND      │         │        │
│              │         │          │         │        │
│ GPIO21 (33)  ├────────►│ TX       │         │        │
│              │         │          │         │        │
│ GPIO22 (36)  │◄────────┤ RX       │         │        │
│              │         │          │         │        │
│              │    GND ─┤ RS       │         │        │
│              │         │          │         │        │
│              │         │ CANH     ├────────►│ CAN_H  │
│              │         │          │         │   |    │
│              │         │ CANL     ├────────►│ CAN_L  │
│              │         │          │         │   |    │
└──────────────┘         └──────────┘         │  120Ω  │ ← Terminator
                                               │   |    │
                                               └───┴────┘
```

### Critical Notes

1. **RS Pin (Slope Control):**
   - **Connect RS to GND** for high-speed mode (supports 500 kbps)
   - If left floating or connected to 3.3V: low-speed mode only (~50 kbps max)

2. **GPIO21/22 Selection Rationale:**
   - ✅ Safe: No boot mode conflicts
   - ✅ Not connected to SPI flash (GPIO6-11)
   - ✅ Not UART0 serial (GPIO1/3)
   - ✅ Not boot mode pins (GPIO0, GPIO2, GPIO12)
   - ✅ Commonly used for I2C/CAN in ESP32 projects

3. **Power:**
   - SN65HVD230 is 3.3V logic
   - ESP32 GPIO outputs 3.3V (compatible)
   - Do NOT use 5V on SN65HVD230 VCC

---

## CAN Bus Termination

### Required Configuration
CAN bus requires **120Ω termination resistors at BOTH physical ends**.

### Terminator Placement

#### End 1: ESP32/SN65HVD230 Side
```
SN65HVD230 CANH ──┬── CAN_H
                  │
                 120Ω  ← Resistor between H and L
                  │
SN65HVD230 CANL ──┴── CAN_L
```

#### End 2: Pi/MCP2515 Side
```
MCP2515 CANH ──┬── CAN_H
               │
              120Ω  ← Resistor between H and L
               │
MCP2515 CANL ──┴── CAN_L
```

### Implementation Options

**Option A: Soldered Resistors**
- Solder 120Ω resistor directly between CANH/CANL on each transceiver module
- Most reliable, permanent solution

**Option B: Screw Terminal**
- Use screw terminal blocks on CAN_H and CAN_L
- Insert resistor into terminal alongside wires

**Option C: Onboard Jumper (if available)**
- Some MCP2515 and SN65HVD230 modules have onboard 120Ω with jumper
- Check your specific modules
- Enable jumper on BOTH ends

**For Short Test Bus (<1m):**
- Single terminator at one end MAY work
- Proper CAN requires two terminators
- Recommend installing both to avoid intermittent issues

---

## Raspberry Pi MCP2515 Configuration

### Current Wiring Status ✅
Already connected per your configuration:

| MCP2515 Pin | Pi GPIO | Pi Pin | Function |
|-------------|---------|--------|----------|
| INT         | GPIO8   | 24     | Interrupt |
| SCK         | GPIO10  | 23     | SPI Clock |
| SI (MOSI)   | GPIO9   | 21     | SPI MOSI |
| SO (MISO)   | GPIO11  | 23*    | SPI MISO |
| CS          | GPIO7   | 26     | SPI Chip Select |
| VCC         | 3.3V    | 1      | Power |
| GND         | GND     | 39     | Ground |

**Note:** You listed "SO → GPIO8 (Pin 23)" but GPIO8 is INT. Assuming MISO is GPIO11 (standard SPI0_MISO).

### Required: `/boot/config.txt` Configuration

**Edit `/boot/config.txt` on Pi:**
```bash
sudo nano /boot/config.txt
```

**Add these lines (if not present):**
```
# Enable SPI
dtparam=spi=on

# MCP2515 CAN Interface
# Adjust oscillator to match your MCP2515 crystal (8MHz or 16MHz)
# interrupt=8 for GPIO8 (Pin 24)
# spimaxfrequency can be tuned (1-10 MHz, 1MHz is conservative)
dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=8,spimaxfrequency=1000000
```

**If your MCP2515 has a 16MHz crystal:**
```
dtoverlay=mcp2515-can0,oscillator=16000000,interrupt=8,spimaxfrequency=1000000
```

**Reboot after editing:**
```bash
sudo reboot
```

**Verify after reboot:**
```bash
dmesg | grep -i mcp251x
# Should see: mcp251x spi0.1 can0: MCP2515 successfully initialized
```

---

## Cable Connection

### CAN Bus Cable (between ESP32 and Pi)

**Minimum 2-wire:**
- CAN_H: SN65HVD230 CANH ↔ MCP2515 CANH
- CAN_L: SN65HVD230 CANL ↔ MCP2515 CANL

**Recommended twisted pair:**
- Use twisted pair cable to reduce EMI
- CAT5/CAT6 works (use one pair)
- Length: Keep <5m for 500 kbps testing

**Common Ground:**
- Ensure ESP32 GND and Pi GND are connected
- Shared power supply OR explicit GND wire between systems
- **Critical:** No common ground = unreliable communication

---

## Verification Checklist

Before powering on, verify:

### ESP32 Side
- [ ] SN65HVD230 VCC connected to ESP32 3.3V (not 5V)
- [ ] SN65HVD230 GND connected to ESP32 GND
- [ ] SN65HVD230 TX connected to GPIO21 (Pin 33)
- [ ] SN65HVD230 RX connected to GPIO22 (Pin 36)
- [ ] SN65HVD230 RS connected to GND (high-speed mode)
- [ ] 120Ω resistor between CANH and CANL

### Raspberry Pi Side
- [ ] MCP2515 already wired to Pi SPI0 ✅
- [ ] `/boot/config.txt` contains MCP2515 overlay
- [ ] Oscillator frequency matches MCP2515 crystal (8MHz or 16MHz)
- [ ] 120Ω resistor between CANH and CANL

### CAN Bus
- [ ] CAN_H connected between both transceivers
- [ ] CAN_L connected between both transceivers
- [ ] ESP32 GND and Pi GND share common ground
- [ ] Total bus length <5m for initial testing

---

## Power-On Sequence

1. **Power ESP32** (via USB or external 5V)
   - Verify: Onboard LED (if any) or serial output

2. **Power Raspberry Pi** (via USB-C or GPIO header)
   - Wait for boot (30-60 seconds)

3. **Check MCP2515 initialization:**
   ```bash
   dmesg | grep -i mcp251x
   # Look for: "MCP2515 successfully initialized"
   ```

4. **Check CAN interface:**
   ```bash
   ip link show can0
   # Should see: can0 interface (may be DOWN, that's OK)
   ```

5. **Bring up CAN interface:**
   ```bash
   sudo ip link set can0 type can bitrate 500000
   sudo ip link set can0 up
   ip link show can0
   # Should show: UP state, bitrate 500000
   ```

---

## Troubleshooting

### ESP32 won't boot after wiring
- **Check:** GPIO0 not pulled LOW (boot mode)
- **Check:** GPIO12 not pulled HIGH at boot
- **Fix:** Disconnect SN65HVD230, verify ESP boots alone, reconnect

### MCP2515 not detected on Pi
- **Check:** SPI enabled in `/boot/config.txt`
- **Check:** Correct oscillator frequency (8MHz vs 16MHz)
- **Run:** `dmesg | grep spi` to see SPI initialization
- **Run:** `ls /dev/spi*` to verify SPI device exists

### CAN interface won't come UP
- **Check:** Termination resistors installed at both ends
- **Check:** CAN_H and CAN_L not swapped
- **Run:** `dmesg | grep can` for kernel messages
- **Try:** Lower bitrate: `sudo ip link set can0 type can bitrate 250000`

### No communication between nodes
- **Verify:** Both transceivers powered
- **Verify:** Common ground between ESP32 and Pi
- **Check:** Termination resistors (measure ~60Ω between CAN_H and CAN_L)
- **Test:** Use `candump can0` on Pi to see traffic
- **Test:** Use `cansend can0 123#DEADBEEF` on Pi to send test frame

---

## Ready to Proceed?

Once wiring is complete:
1. Follow **Power-On Sequence** above
2. Proceed to roadmap **Phase 3.1** (ESP32 TWAI init)
3. Proceed to roadmap **Phase 4.1** (Pi SocketCAN config)

**All hardware decisions finalized. Ready for implementation.**
