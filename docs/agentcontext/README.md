# Agent Context Directory

This directory contains metadata and tracking information for agent-driven development.

## Contents

- **version_log.md**: Project-wide changelog of all agent modifications
  - Tracks version numbers, user requests, files modified, and planning docs
  - Updated after major changes or task completion
  - Provides audit trail and context for future work

## Purpose

This directory helps:
- Track evolution of the codebase over time
- Maintain context across agent sessions
- Provide humans with a clear history of what changed and why
- Enable agents to understand past decisions before making new changes

## Usage

Before starting work, agents should:
1. Read `version_log.md` to understand recent changes
2. Use the latest version number to determine next version tag
3. After completing work, add new entry to version log

Humans can:
- Review the version log to understand project history
- Reference version tags in code to trace back to original requests
- Use this as a basis for rollback or debugging decisions
