# GD Replay Tools

A Geode mod project targeting Geometry Dash on Android ARM64.

## Features planned

- Bot Replay: record and replay timestamped input events.
- Speedhack: configurable gameplay speed multiplier.
- Noclip: collision bypass for practice.
- Jump Hack: configurable jump assistance.
- Visuals: title-screen background customization.

## Current source status

- Geode project scaffold and metadata.
- Settings for the planned practice and visual features.
- Input recorder prototype captures button press and release events with microsecond timestamps.
- Recorded events are serialized as CSV text and stored in Geode saved data when leaving a level.

Playback, dedicated macro-file export/import, Speedhack, Noclip, Jump Hack, and background replacement are not implemented yet.

## Android ARM64 CI

A GitHub Actions workflow builds the project for Android64 and uploads a `.geode` artifact when the workflow succeeds.

Open the repository's **Actions** tab to inspect the build result and download the artifact.

## Target compatibility

The requested target is Android ARM64. The user-reported game version is `2.2.144`; this needs confirmation against the exact version string exposed by the installed Geode loader. The project has not yet been verified on the user's phone.
