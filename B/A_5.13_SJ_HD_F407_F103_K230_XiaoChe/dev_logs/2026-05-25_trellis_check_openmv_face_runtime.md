# Trellis Check For OpenMV Face Runtime

- Date: `2026-05-25`
- Status: `completed`
- Scope:
  - `OpenMV Visual module(SJ)\code\main.py`
  - `.trellis\spec\backend\quality-guidelines.md`
  - `.trellis\spec\backend\logging-guidelines.md`
- Trigger:
  - Run the `trellis-check` quality verification workflow after recent OpenMV
    face-tracking stabilization changes.
- Step summary:
  - Reviewed workspace change state with `git diff --name-only HEAD` and
    `git status --short`.
  - Determined the repo has no package-level `.trellis` context and the
    existing spec files are generic placeholders.
  - Read relevant `.trellis` guidance:
    - `backend/index.md`
    - `backend/quality-guidelines.md`
    - `backend/logging-guidelines.md`
    - `guides/code-reuse-thinking-guide.md`
  - Ran available host-side validation for the edited OpenMV script:
    - `python -m py_compile OpenMV Visual module(SJ)\code\main.py`
- Findings:
  - No Python syntax errors remain in `main.py`.
  - No project-specific lint, type-check, or test command exists for this
    OpenMV script area, so those checks could not be run.
  - A non-obvious board-specific issue was confirmed:
    - `image.HaarCascade("frontalface")` is not reliable on the current N6
      firmware in use.
    - The explicit ROM path `/rom/haarcascade_frontalface.cascade` is the
      safer fallback strategy.
  - Default runtime logging is now aligned with stability requirements:
    - `PRINT_FPS_TO_REPL = False`
    - `PRINT_VISION_TO_REPL = False`
- Spec sync:
  - Updated `.trellis` backend quality guidelines with embedded/OpenMV runtime
    quality constraints.
  - Updated `.trellis` backend logging guidelines with bounded-rate logging
    rules for camera control loops.
- Residual risk:
  - Runtime stability on real hardware still depends on LCD bus behavior and
    power integrity; host-side checks cannot prove that dimension.
