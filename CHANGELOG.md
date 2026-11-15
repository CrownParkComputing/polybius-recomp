# Polybius port changes

## 0.1 — 2026-11-15

- Add a separate Linux and Android port for CUSA08328 v1.03 using the shared
  PS4 runtime, L3+R3 internal menu and graphics-driver selector.
- Verify Linux level 1 rendering, game-over return, non-silent audio output
  and save-file creation. Steady sections run at 60 FPS with compilation pauses.
- Build and install the separate Android app with bundled Turnip and ZIP import;
  device gameplay validation remains pending.
- Include source-only CI, build instructions and explicit verification limits.
