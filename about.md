# GD Replay Tools

A Geometry Dash practice and customization mod for Geode.

## Planned features

- **Bot Replay:** timestamped input recording and playback.
- **Speedhack:** adjustable gameplay speed for practice.
- **Noclip:** collision bypass for practice/testing.
- **Jump Hack:** configurable jump assistance.
- **Visuals:** customizable title-screen background and other visual options.

## Development status

This is an early source scaffold. The Geode entry point and project metadata are in place; gameplay hooks, replay serialization, settings UI, and visual customization are not implemented yet.

The project targets Android ARM64. The Android game version is currently left broad in metadata until the exact version string expected by the installed Geode SDK is confirmed.

## Build

Install the Android NDK, configure the ANDROID_NDK_ROOT environment variable, install the matching Geode Android binaries, then run:

    geode build -p android64

A successful build and in-game testing are required before release.
