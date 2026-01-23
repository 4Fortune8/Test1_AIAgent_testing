# Agent Version Log

This file tracks major changes and task completions made by agents to the codebase.

## Version Format
Versions follow the pattern: **V{major}.{step}.{query}**

- **Major**: Significant feature or architectural change (V1.xx.xx → V2.xx.xx)
- **Step**: Sub-task or component within a major change (V1.00.xx → V1.01.xx)
- **Query**: Individual query/interaction index within a step (V1.00.01 → V1.00.02)

Examples:
- `V1.00.01` - First query of first step in major version 1
- `V1.01.00` - Second step of major version 1
- `V2.00.00` - Major architectural change (new major version)

## Format
Each entry should include:
- **Version**: V{major}.{step}.{query}
- **Date**: YYYY-MM-DD
- **User Request**: Brief summary of what was requested
- **Files Modified**: List of changed files
- **Planning Document**: Reference to thought process document
- **Status**: Completed / In Progress / Rolled Back

---

## Version History

### V0.00.00 - 2026-01-22
**User Request**: Initial project setup  
**Files Modified**: Project structure created  
**Planning Document**: N/A  
**Status**: Completed  
**Notes**: Base project structure established with ESP, Pi, protocol, and docs directories.

---

### V1.00.00 - 2026-01-22
**User Request**: "Create detailed ROADMAP for minimal PING/PONG CAN communication"  
**Files Modified**:
- `docs/agentcontext/roadmap_can_ping_pong.md` (created, ~650 lines)
- `docs/agentcontext/checklist_can_ping_pong.md` (created)
- `docs/agentcontext/protocol_amendment_pong.md` (created)
- `protocol/can_id_map.md` (updated with complete ID tables)
- `protocol/can_invariants.md` (created, 15 binding rules)

**Planning Document**: Roadmap itself serves as planning document  
**Status**: Completed  
**Notes**: 
- PONG ID allocated: 0x111 (STATUS class, 4-byte payload)
- Protocol enhancements: sequence counters, invariants, filters
- Initial GPIO: GPIO21/22 (later changed to GPIO5/4)

---

### V2.00.00 - 2026-01-22
**User Request**: "Configure for ESP32-WROOM-32D with GPIO5/4 based on YAML specification"  
**Files Modified**:
- `docs/agentcontext/roadmap_can_ping_pong.md` (8 sections updated)
- `docs/agentcontext/hardware_wiring_guide.md` (created, ~350 lines)
- `docs/agentcontext/checklist_can_ping_pong.md` (updated)
- `docs/agentcontext/version_log.md` (this entry)

**Planning Document**: `docs/agentcontext/GPIO_UPDATE_v2.md` (transition doc)  
**Status**: Completed  
**Notes**: 
- **GPIO change: GPIO21/22 → GPIO5/4** (safe defaults, no boot conflicts)
- **Pi SPI corrected**: MOSI=GPIO10, MISO=GPIO9, SCLK=GPIO11, CS=GPIO8, INT=GPIO25
- **RS pin**: Must be grounded for high-speed mode (up to 1 Mbps)
- **Termination**: 120Ω at both ends required (measures ~60Ω total)
- `/boot/config.txt` updated: `interrupt=25` for GPIO25

---

## Template for New Entries

### V#.##.## - YYYY-MM-DD
**User Request**: [Brief description of user request]  
**Files Modified**:
- `path/to/file1.cpp`
- `path/to/file2.py`

**Planning Document**: `path/to/thoughtprocesses/YYYY-MM-DD_description.md`  
**Status**: [Completed/In Progress/Rolled Back]  
**Notes**: [Any important context, gotchas, or follow-up needed]

---
