# Phase 40

- Date: `2026-05-25`
- Topic: `trellis_check_openmv_face_runtime`
- Trigger:
  - The `trellis-check` skill was invoked after a sequence of OpenMV face
    tracking fixes.
- Result:
  - The recent OpenMV runtime changes were reviewed with the repository
    quality-check workflow.
  - Host-side syntax validation passed for:
    - `OpenMV Visual module(SJ)\code\main.py`
  - No repository-local lint / type-check / test pipeline exists for this
    script area, so only applicable checks were run.
  - The `.trellis` backend quality and logging specs were updated to capture
    the non-obvious OpenMV runtime lessons from this debugging cycle.
- Main lesson captured:
  - Avoid per-frame REPL logging in OpenMV control loops.
  - Prefer explicit ROM resource paths for Haar cascades on the current N6
    firmware when alias loading is unreliable.
