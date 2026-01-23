# CAN Bus Hardware Wiring Guide (Updated)
**Date:** 2026-01-22 (v2)  
**System:** ESP32-WROOM-32D ↔ Raspberry Pi 3B+ CAN Communication  
**Status:** ✅ WIRING COMPLETE - Ready for Software Configuration  
**GPIO Configuration:** GPIO5 (TX) / GPIO4 (RX) - Safe defaults

---

## Components Verified

### ESP32 Side
- **Board:** ESP32-WROOM-32D on UPeasy devkit
- **CAN Transceiver:** SN65HVD230 (3.3V logic)
- **Selected Pins:** **GPIO5 (TX), GPIO4 (RX)** - Safe defaults, no boot conflicts

### Raspberry Pi Side
- **Board:** Raspberry Pi 3B+
- **CAN Transceiver:** MCP2515 CAN Bus Module (SPI interface)
- **Status:** ✅ Wired to Pi SPI0, `can0` interface UP
- **SSH Access:** `ssh fabricpi@10.42.0.48` (password: fabricpi)

---

## 1. ESP32-WROOM-32D to SN65HVD230 Wiring

| ESP32-WROOM-32D Pin | GPIO | Signal | → | SN65HVD230 Pin | Notes |
|---------------------|------|--------|---|----------------|-------|
| Pin 29              | GPIO5 | CAN TX | → | TX (D)        | Transmit data to transceiver |
| Pin 26              | GPIO4 | CAN RX | → | RX (R)        | Receive data from transceiver |
| Pin 2               | 3V3   | Power  | → | 3V3 (VCC)     | 3.3V supply only (NOT 5V) |
| Pin 1, 15, or 38    | GND   | Ground | → | GND           | Common ground |
| —                   | —     | —      | → | RS            | Connect RS to GND for high-speed mode (max bitrate) |

**Critical Notes:**
- **GPIO5/4 are safe:** No boot mode conflicts, not SPI flash pins (GPIO6-11), not UART0 (GPIO1/3)
- **3.3V only:** SN65HVD230 is a 3.3V device; connecting 5V will damage it
- **RS pin:** Must be grounded (GND) for high-speed CAN (up to 1 Mbps)

### ESP32 Pin Reference (ESP32-WROOM-32D)
```
GPIO4  → Pin 26 (CAN RX)
GPIO5  → Pin 29 (CAN TX)
3V3    → Pin 2  (Power)
GND    → Pin 1, 15, or 38 (Ground)
```

**Why GPIO5/4?**
- NOT boot mode pins (GPIO0, GPIO2, GPIO12)
- NOT SPI flash pins (GPIO6-11, would brick device)
- NOT UART0 (GPIO1/3, used for programming/debug)
- Tested safe defaults for CAN communication

---

## 2. Raspberry Pi 3B+ to MCP2515 Wiring

| MCP2515 Pin | Signal | → | Pi Physical Pin | Pi GPIO | Notes |
|-------------|--------|---|-----------------|---------|-------|
| VCC         | Power  | → | Pin 1 (or 17)   | 3.3V    | 3.3V supply only (NOT 5V) |
| GND         | Ground | → | Pin 39 (or any) | GND     | Common ground |
| SI (MOSI)   | SPI    | → | Pin 19          | GPIO10  | SPI0_MOSI |
| SO (MISO)   | SPI    | → | Pin 21          | GPIO9   | SPI0_MISO |
| SCK         | SPI    | → | Pin 23          | GPIO11  | SPI0_SCLK |
| CS          | SPI    | → | Pin 24          | GPIO8   | SPI0_CE0 (Chip Select) |
| INT         | Interrupt | → | Pin 22       | GPIO25  | MCP2515 interrupt to Pi |

**Critical Notes:**
- **3.3V only:** MCP2515 modules are typically 3.3V; verify before connecting
- **SPI0 interface:** Pi has two SPI buses; we use SPI0 (GPIO8-11)
- **INT pin:** GPIO25 used for interrupt; must match `/boot/config.txt` overlay parameter

---

## 3. CAN Bus Connections

### Physical CAN Bus Wiring
```
SN65HVD230 (ESP32 side)          MCP2515 (Pi side)
┌──────────┐                     ┌──────────┐
│  CANH    ├─────────────────────┤  CANH    │
│          │   (twisted pair)    │          │
│  CANL    ├─────────────────────┤  CANL    │
└──────────┘   max 5m for test   └──────────┘
    |                                  |
   120Ω                               120Ω
(Terminator #1)                  (Terminator #2)
```

**Termination Resistors:**
- **Required:** One 120Ω resistor at EACH physical end
- **Location:** Between CAN_H and CAN_L at both ESP32/SN65HVD230 and Pi/MCP2515
- **Verification:** Measure resistance between CAN_H and CAN_L with both terminators installed → should read **~60Ω** (two 120Ω in parallel)
- **Short-term testing:** Can use single terminator if bus < 1m, but two is proper

---

## 4. Complete System Diagram

```
        ESP32-WROOM-32D                                   Raspberry Pi 3B+
      (UPeasy devkit)                                                     
                                                                          
      Pin 29 (GPIO5) ────────┐                            Pin 1 (3.3V) ──┐
                              │                                           │
      Pin 26 (GPIO4) ───────┐ │                           Pin 39 (GND) ──┼──┐
                            │ │                                           │  │
      Pin 2 (3.3V) ─────────┼─┼──┐                        Pin 19 (GPIO10)│  │
                            │ │  │                         ↓             │  │
      Pin 1/15/38 (GND) ────┼─┼──┼────────────────────────────────────────┼──┼──── Common GND
                            │ │  │                                        │  │
                            ↓ ↓  ↓                                        ↓  ↓
                        ┌──────────┐                                  ┌─────────┐
                        │SN65HVD230│                                  │ MCP2515 │
                        │  (3.3V)  │                                  │  (SPI)  │
                        │          │                                  │         │
                        │ TX ←─────┤ GPIO5                            │ VCC ←───┤ 3.3V
                        │ RX ←─────┤ GPIO4                            │ GND ←───┤ GND
                        │ VCC ←────┤ 3.3V                             │ SI ←────┤ GPIO10
                        │ GND ←────┤ GND                              │ SO ─────→ GPIO9
                        │ RS ←─────┤ GND (high-speed)                 │ SCK ←───┤ GPIO11
                        │          │                                  │ CS ←────┤ GPIO8
                        │ CANH ────┼───── twisted ──────────────────  │ CANH ───┤ INT ←── GPIO25
                        │          │      pair                        │         │
                        │ CANL ────┼───── (~5m max) ─────────────────  │ CANL    │
                        │          │                                  │         │
                        │  [120Ω]  │                                  │  [120Ω] │
                        └──────────┘                                  └─────────┘
                       Terminator #1                                 Terminator #2
                     (both ends required)                          (measures ~60Ω total)
```

---

## 5. Raspberry Pi Software Configuration

### SSH Connection
```bash
ssh fabricpi@10.42.0.48
# Password: fabricpi
```

### Configure `/boot/firmware/config.txt`

```bash
# Edit the config file (requires sudo)
sudo nano /boot/firmware/config.txt

# Add or verify these lines:
dtparam=spi=on
dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25,spimaxfrequency=1000000

# Where:
#   oscillator=8000000 → For 8 MHz crystal (change to 16000000 if 16 MHz)
#   interrupt=25 → GPIO25 (Pin 22) connected to MCP2515 INT
#   spimaxfrequency=1000000 → Conservative 1 MHz SPI (can increase to 10 MHz)

# Save and reboot
sudo reboot
```

**Critical:**
- `oscillator` parameter MUST match your MCP2515 crystal frequency (8 or 16 MHz)
- `interrupt` parameter MUST match GPIO pin connected to MCP2515 INT (GPIO25)
- Wrong oscillator frequency → no CAN communication
- Modern Pi OS uses `/boot/firmware/config.txt` (not `/boot/config.txt`)

---

## 6. Power-On Sequence

1. **Verify all connections** (use checklist below)
2. **Power on Raspberry Pi first** (allows SocketCAN to initialize)
3. **Power on ESP32** (TWAI driver will initialize on boot)
4. **Check MCP2515 initialization:** `dmesg | grep mcp251x`
5. **Bring up can0 interface:** `sudo ip link set can0 up type can bitrate 500000`
6. **Verify interface:** `ip link show can0` (should show UP state)
7. **Check for errors:** `ip -s link show can0` (TX/RX errors should be 0)

---

## 7. Troubleshooting

### ESP32 Issues
| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| ESP32 won't boot | GPIO5/4 wiring error | Disconnect CAN wires, verify boot |
| TWAI init fails | Wrong GPIO pins in code | Check `#define CAN_TX_GPIO 5` / `CAN_RX_GPIO 4` |
| No CAN traffic | SN65HVD230 not powered | Verify 3.3V supply to transceiver |
| Bus-off errors | Termination missing | Install 120Ω resistors at both ends |

### Raspberry Pi Issues
| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| `can0` not visible | MCP2515 overlay missing | Check `/boot/firmware/config.txt`, reboot |
| `mcp251x` not loaded | SPI not enabled | Add `dtparam=spi=on` to config |
| Interrupt errors | Wrong GPIO in overlay | Verify `interrupt=25` matches wiring |
| Bitrate errors | Wrong oscillator freq | Match `oscillator=` to crystal (8/16 MHz) |
| Probe failed (err=110) | Oscillator mismatch or wiring | Try 16MHz if 8MHz fails, check SPI wiring |

### CAN Bus Issues
| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| High error rates | Missing termination | Measure ~60Ω between CAN_H/L |
| No communication | CAN_H/L swapped | Swap CANH and CANL on one end |
| Intermittent errors | Shared ground missing | Connect ESP32 GND to Pi GND |
| Bus-off condition | Cable too long | Reduce to <5m for 500 kbps |

---

## 8. Verification Checklist

### ESP32 Wiring
- [ ] GPIO5 (Pin 29) → SN65HVD230 TX
- [ ] GPIO4 (Pin 26) → SN65HVD230 RX
- [ ] 3.3V (Pin 2) → SN65HVD230 VCC
- [ ] GND (Pin 1/15/38) → SN65HVD230 GND
- [ ] SN65HVD230 RS → GND (for high-speed mode)

### Raspberry Pi Wiring
- [ ] GPIO10 (Pin 19) → MCP2515 SI (MOSI)
- [ ] GPIO9 (Pin 21) → MCP2515 SO (MISO)
- [ ] GPIO11 (Pin 23) → MCP2515 SCK
- [ ] GPIO8 (Pin 24) → MCP2515 CS
- [ ] GPIO25 (Pin 22) → MCP2515 INT
- [ ] 3.3V (Pin 1) → MCP2515 VCC
- [ ] GND (Pin 39) → MCP2515 GND

### CAN Bus Wiring
- [ ] SN65HVD230 CANH → MCP2515 CANH (twisted pair cable)
- [ ] SN65HVD230 CANL → MCP2515 CANL (twisted pair cable)
- [ ] 120Ω resistor between CANH/CANL at ESP32 end
- [ ] 120Ω resistor between CANH/CANL at Pi end
- [ ] Measured resistance ~60Ω between CAN_H and CAN_L

### Shared Ground
- [ ] ESP32 GND connected to Pi GND (mandatory for reliable CAN)
- [ ] All devices powered from stable supplies
- [ ] No ground loops (single GND path preferred)

### Raspberry Pi Configuration
- [ ] `/boot/firmware/config.txt` contains `dtparam=spi=on`
- [ ] `/boot/firmware/config.txt` contains `dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25`
- [ ] Rebooted after editing `/boot/firmware/config.txt`
- [ ] Modules loaded: `lsmod | grep mcp251x` shows loaded
- [ ] Interface visible: `ip link show can0` shows device

### Pre-Test Verification
- [ ] No loose connections
- [ ] No shorts between VCC and GND
- [ ] Termination resistors correctly installed
- [ ] GPIO pins match firmware configuration (GPIO5/4)
- [ ] `/boot/firmware/config.txt` interrupt parameter matches wiring (GPIO25)

---

## 9. Next Steps

After completing physical wiring and verification:

1. **Create ESP planning document:** `esp/thoughtprocesses/2026-01-22_can-bringup-esp.md`
2. **Create Pi planning document:** `pi/thoughtprocesses/2026-01-22_can-bringup-pi.md`
3. **Create protocol planning:** `protocol/thoughtprocesses/2026-01-22_ping-pong-spec.md`
4. **Implement ESP TWAI initialization** with GPIO5/4 configuration
5. **Test PING/PONG communication** per roadmap

---

## 10. Reference Documents

- **Main Roadmap:** `docs/agentcontext/roadmap_can_ping_pong.md`
- **Protocol Specification:** `protocol/can_id_map.md`, `protocol/can_invariants.md`
- **PONG Command Spec:** `docs/agentcontext/protocol_amendment_pong.md`
- **Implementation Checklist:** `docs/agentcontext/checklist_can_ping_pong.md`

---

**End of Wiring Guide**
