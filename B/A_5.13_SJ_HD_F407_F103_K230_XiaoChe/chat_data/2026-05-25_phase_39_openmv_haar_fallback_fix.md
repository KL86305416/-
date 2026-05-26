# Phase 39

- Date: `2026-05-25`
- Topic: `openmv_haar_fallback_fix`
- Trigger:
  - The board reported:
    - `haar load failed: [Errno 2] ENOENT`
- Result:
  - The Haar fallback loader in `OpenMV Visual module(SJ)\code\main.py` was changed from the short built-in alias to an ordered compatibility load strategy.
  - It now tries:
    - `/rom/haarcascade_frontalface.cascade`
    - `frontalface`
  - This matches the local official OpenMV face-detection examples more closely.
  - The primary `BlazeFace` detector path was left unchanged.
- Verification:
  - A new backup was saved to:
    - `backups/2026-05-25_openmv_n6_haar_fallback_fix`
  - Host-side `py_compile` passed.
