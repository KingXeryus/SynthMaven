# SM FM1 Editor — 3.5 development

This USB-first app bundles the same editor and protocol handlers as `web/editor.html`.
It retains the existing SynthMaven CoreMIDI transport, bounded SysEx framing, endpoint
matching, native connection wizard, and supplied app icon. It does not inject the
old SynthMaven 3.0 parameter bindings into the Melodee-based protocol.

Run `python3 ios/generate_project.py`, then `ios/test.sh`. The tests build an unsigned
iOS Simulator application; they do not install firmware or distribute TestFlight.
The current target is version 3.5, build 11, home-screen name **SM FM1 Editor**.

## Connecting

1. Set the FM-1 USB role to COMPUTER.
2. Connect a USB data cable, with an adapter or powered hub when required by the iPad.
3. Choose Connect → USB cable. Matching input and output ports are selected together.
4. The editor reads the firmware capabilities before enabling supported controls.

Only the matching 3.5 development firmware should be used for migration validation.
A port name such as “Melodee FM-1” is not sufficient evidence of its protocol version.
Direct Bluetooth is excluded from this candidate and has its own later validation gate.

## Work still required before distribution

- Shared touch-first Perform view and final responsive layout verification.
- Physical USB parameter editing, disconnect/reconnect and export/import checks.
- Firmware/controller integration validation and release guide.
- Archive/signing, App Store Connect build-number check, and internal/external testing approval.

No TestFlight upload or external invitation has been made from this branch.
