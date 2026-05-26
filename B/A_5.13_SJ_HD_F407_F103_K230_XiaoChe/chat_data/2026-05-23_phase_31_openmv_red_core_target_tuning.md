# Phase 31

- Date: `2026-05-23`
- Topic: `openmv_red_core_target_tuning`
- Clarification:
  - The user confirmed the target object is a transparent ring with a red center disc, not a simple solid red circle.
- Result:
  - The active OpenMV classifier now prefers a large round red core, tolerates the white printed logo inside it, and ignores the transparent outer ring.
  - The protocol and F407 receive side were left unchanged.
