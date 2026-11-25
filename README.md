# GD Replay Tools

Geode mod starter targeting Geometry Dash 2.2 on Android ARM64.

## Planned modules

- Bot Replay: record and replay timestamped player inputs.
- Speedhack: configurable game speed multiplier.
- Noclip: collision bypass for practice/testing.
- Jump Hack: configurable jump assistance.
- Visuals: customizable menu and title-screen background.

## Target compatibility

- Platform: Android ARM64
- Geometry Dash: user-reported version 2.2.144
- Mod loader: Geode

Compatibility must be verified against the exact installed game and Geode SDK versions.

## Build

With the Geode SDK/toolchain installed:

```sh
geode build -p android64
```

This repository currently contains the project plan only. A successful Android build and in-game tests are required before calling the mod complete.
