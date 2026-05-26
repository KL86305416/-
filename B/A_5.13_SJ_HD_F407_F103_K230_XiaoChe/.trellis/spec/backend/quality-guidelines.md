# Quality Guidelines

> Code quality standards for backend development.

---

## Overview

This repository contains MCU firmware, OpenMV MicroPython scripts, local
documentation mirrors, and project records. For embedded runtime code, the
quality bar is defined first by sustained runtime stability on target hardware,
then by host-side syntax validation, and then by documentation consistency.

---

## Forbidden Patterns

- Per-frame REPL / serial debug logging in OpenMV runtime loops unless it is
  deliberately rate-limited and temporary.
- Repeated dynamic buffer allocation inside high-frequency camera loops when a
  fixed buffer can be reused.
- Assuming ROM aliases and helper shortcuts behave the same across OpenMV
  firmware variants without validating them on the actual board.

---

## Required Patterns

- Before changing embedded constants or protocol fields, search the repo for
  all consumers and producers.
- For OpenMV runtime scripts:
  - prefer bounded-rate logging
  - prefer preallocated frame / packet buffers in hot loops
  - add graceful fallback paths when model or cascade loading fails
- When fixing a board-specific runtime issue, capture the exact failure mode in
  `dev_logs/` and `chat_data/`.

---

## Testing Requirements

- Host-side syntax checks are mandatory for edited Python scripts.
- Hardware-facing behavior changes should document the minimum on-board
  verification needed, even if the hardware is not currently connected.
- If no automated lint / type-check / test command exists for the affected
  area, state that explicitly in the verification report instead of silently
  skipping it.

---

## Code Review Checklist

- Does the runtime loop avoid obvious allocation churn?
- Are debug prints disabled or rate-limited by default?
- Are fallback resource paths explicit where firmware aliasing is unreliable?
- Does the change preserve the existing serial protocol unless a coordinated
  cross-layer change is intended?
