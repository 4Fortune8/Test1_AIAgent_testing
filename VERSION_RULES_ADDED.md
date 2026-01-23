# Version Numbering Rules Added

**Date:** 2026-01-22  
**Status:** ✅ Complete

---

## Summary

Added strict version numbering rules to control when agents can increment major versions. Major version changes now **require explicit user approval**.

---

## What Was Changed

### 1. Added Version Rules to `.github/chatmodes/copilot-instructions.chatmode.md` ✅

Added comprehensive section explaining:
- **Major Version (V1 → V2)**: Requires explicit user approval
  - Major architectural changes
  - Major feature additions
  - Breaking changes to interfaces
  - Protocol version changes
  - Agent MUST ask before incrementing

- **Step Version (V1.00 → V1.01)**: Agent discretion
  - New sub-features within current major version
  - Configuration updates
  - Hardware specification changes
  - New modules within existing architecture

- **Query Version (V1.00.00 → V1.00.01)**: Automatic
  - Individual user interactions
  - Minor tweaks/fixes
  - Documentation-only updates
  - Bug fixes

### 2. Fixed Version Log (`docs/agentcontext/version_log.md`) ✅

Corrected version numbers that were incorrectly incremented:
- **V2.00.00 → V1.01.00** (GPIO configuration change - step increment, not major)
- **V2.01.00 → V1.02.00** (PlatformIO configuration - step increment, not major)

Added notes explaining the corrections.

---

## Version Numbering Guidelines

### Most Common Pattern (Expected)
```
V1.00.00 - Initial feature/roadmap
V1.00.01 - Small update (query increment)
V1.00.02 - Another small update
V1.01.00 - New sub-feature (step increment)
V1.01.01 - Fix/tweak in that sub-feature
V1.02.00 - Another sub-feature
V1.02.01 - Documentation update
```

### Major Version (Rare - Requires User Approval)
```
V1.xx.xx - Working on CAN communication system
↓
[Agent asks: "Should I increment to V2.00.00 for motor control addition?"]
↓
[User approves]
↓
V2.00.00 - Motor control system added (major feature)
```

---

## Key Rules for Agents

1. **Default behavior**: Increment **step** or **query**, NOT major
2. **Configuration changes** (GPIO pins, settings): Step increment (V1.xx → V1.yy)
3. **Documentation only**: Query increment (V1.00.xx → V1.00.yy)
4. **New modules/features within current scope**: Step increment
5. **Major architectural changes**: STOP and ask user for approval before incrementing to V2

---

## Current Project Status

**Current Version:** V1.02.00
- V1.00.00 - Initial CAN PING/PONG roadmap
- V1.01.00 - GPIO5/4 configuration
- V1.02.00 - PlatformIO configuration

**Next likely increments:**
- V1.02.01 - Documentation tweaks
- V1.03.00 - File cleanup (completed earlier today)
- V1.04.00 - Next sub-feature (TWAI driver implementation, etc.)

**Next major version would be:** V2.00.00
- Requires: User approval
- Example trigger: Adding motor control, switching to different transport protocol, major system architecture change

---

## Benefits

1. ✅ **Prevents version inflation**: Major versions stay meaningful
2. ✅ **Clear milestones**: V1 vs V2 represents real architectural differences
3. ✅ **User control**: Humans approve significant changes
4. ✅ **Agent guidance**: Clear rules for when to increment what
5. ✅ **Better history**: Version numbers tell the story of the project

---

## Example Scenarios

| Change | Old (Wrong) | New (Correct) |
|--------|-------------|---------------|
| GPIO pin change | V1→V2 | V1.00→V1.01 |
| PlatformIO config | V2→V3 | V1.01→V1.02 |
| Documentation fix | V1.00→V1.01 | V1.00.00→V1.00.01 |
| Add motor control | V1→V2 (no ask) | V1→V2 (with user approval) ✅ |
| Protocol change | V1→V2 (no ask) | V1→V2 (with user approval) ✅ |

---

**Rules are now in place. Agents will ask before major version changes.** 🎯
