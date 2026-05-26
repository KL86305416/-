# Phase 29

- Date: `2026-05-23`
- Topic: `openmv_n6_blob_api_fix`
- Trigger:
  - After removing the `machine.PWM` dependency, the script advanced further but then failed in `classify_red_circle()` because blob fields were callable methods on the actual board runtime.
- Result:
  - Active `main.py` now reads blob geometry and scoring fields through a compatibility layer that supports both property-style and method-style access.
  - The red-circle detection, display overlay, and `UART7` packet transmission path remain unchanged in behavior.
- Current next step:
  - re-run on the N6 and check whether the loop now runs continuously on real hardware
