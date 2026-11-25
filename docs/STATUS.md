# Development status

## Implemented in source

- Geode mod entry point.
- Geode settings for input recording, speed multiplier, noclip, jump option, and menu visuals.
- A first input-recorder prototype that records timestamped PlayerButton press events in memory while recording is enabled.

## Not implemented yet

- Persisting recordings to a file.
- Playback and deterministic replay.
- Button-release capture and timing validation.
- Actual speed manipulation.
- Collision bypass.
- Jump behavior changes.
- Main-menu background replacement.

## Important compatibility note

The target is Android ARM64. The game version supplied so far is "2.2.144"; confirm the exact version shown by Geode because Geode's official metadata commonly uses a numeric game version format such as 2.2081. Do not assume compatibility based on the APK filename.

## Validation

No Android build has been run from this repository yet. The source must be compiled against the Geode SDK version installed for the target game, then tested on-device.
