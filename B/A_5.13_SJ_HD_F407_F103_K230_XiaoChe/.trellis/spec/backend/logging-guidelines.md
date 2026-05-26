# Logging Guidelines

> How logging is done in this project.

---

## Overview

Most runtime targets in this repository do not have a rich logging stack.
Logging is usually plain `print()` on OpenMV MicroPython or serial text on MCU
firmware. Because runtime bandwidth is limited, logging policy must optimize
for non-interference with control loops.

---

## Log Levels

- `error`: one-time or low-frequency failure to load a model, cascade, or
  hardware peripheral.
- `debug`: temporary development diagnostics only; must be disabled by default
  or rate-limited.

---

## Structured Logging

- OpenMV scripts may use compact plain-text status lines.
- If loop telemetry is printed, it must be emitted at a bounded interval rather
  than once per frame.

---

## What to Log

- Resource initialization failures that change the active detection path.
- Persistent runtime failures that force a degraded mode.
- Enough context to reproduce the issue on the same board and firmware.

---

## What NOT to Log

- Per-frame telemetry in steady-state control loops.
- High-volume messages that can block the OpenMV IDE / REPL transport.
- Redundant success spam after a resource has already initialized.
