# Development status

## Implemented in source

- Geode mod entry point.
- Settings for recording, speed multiplier, noclip, jump option, and menu visuals.
- Timestamped capture of PlayerObject button press and release events.
- Replay event serialization to CSV text stored in Geode saved data when PlayLayer is quit.
- Android64 GitHub Actions build workflow.

## Not implemented yet

- Loading a saved replay into a playback timeline.
- Replaying events at their recorded timestamps.
- Dedicated .gdbot file import/export.
- Actual speed manipulation.
- Collision bypass.
- Jump behavior modification.
- Main-menu background replacement.

## Compatibility and validation

Target: Android ARM64. The game version reported by the user is "2.2.144"; verify the exact version displayed by the installed Geode loader and use the matching SDK/bindings.

The GitHub Actions workflow has been added but has not yet been confirmed successful. No in-game test has been performed on the user's phone.
