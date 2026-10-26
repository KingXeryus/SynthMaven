# SynthMaven 0.5 — iPhone/iPad app and AUv3

The iOS targets share the Mac app/VST3/AU processor and editor: eight-layer
mixer plus master, per-layer octave/split controls, wave sampler, MIDI mapping,
Scala/custom tuning and the factory library. The iOS minimum target is 15.0. The preparation script applies a single-scene
UIKit lifecycle bridge to the pinned JUCE source for current SDK compatibility.
The mixer/sampler can scroll horizontally on narrow screens; Library opens the
browser on compact layouts. iPad has more room for the nine channel strips.

## Bluetooth MIDI

Open **MIDI MAP → BLUETOOTH MIDI** to show Apple's native MIDI pairing panel.
Allow Bluetooth access and connect a compatible MIDI keyboard/pedal. The iOS
standalone automatically discovers/enables newly connected Core MIDI inputs;
layer channel/split filtering and MIDI Learn then apply as usual.

In AUv3, the DAW owns MIDI input: pair the device, then route its MIDI to the
SynthMaven instrument in the host. A BLE device appearing in the panel does not
by itself establish that the host is forwarding its notes/controllers.
Bluetooth usage descriptions are included for the app/extension, and background
audio is enabled. Real Bluetooth pairing/latency requires an iPhone/iPad and
compatible hardware; simulator compilation does not validate radio performance.

## Import and save samples

Use **SAMPLER → IMPORT AUDIO** and the Files picker. Mono/stereo audio up to 30
seconds at 8–192 kHz is accepted; imported audio is converted to 44.1 kHz and
embedded as 24-bit WAV in instrument state. DAW sessions and `.synthmaven` presets
retain the audio independently of the original file. Sharing large eight-sample
presets increases project size. Only use samples you are permitted to use.

## Build

Install full Xcode and the iOS SDK. Run from the repository root:

```sh
CMAKE=/path/to/cmake PYTHON=/path/to/python3 bash desktop/tools/build-ios.sh simulator
CMAKE=/path/to/cmake PYTHON=/path/to/python3 bash desktop/tools/build-ios.sh device
```

The device command defaults to an **unsigned** build, suitable for compilation
validation. It is not an installable, signed IPA. To sign for your device,
configure an active Apple Developer team and provisioning in Xcode, or supply
`SYNTHMAVEN_TEAM` to the device script and complete Xcode's account/provisioning
steps. Standalone embeds the AUv3 extension. App Store/TestFlight distribution
requires the appropriate signing, provisioning, review and account setup.

No developer credentials are embedded. The Mac installer/DMG cannot install
an iOS app. See [validation status](VALIDATION.txt) for the builds actually tested.

## Tuning library
The shared tuning page has a scale-degree keyboard with detuning highlights, 34 canonical maqam names and independent root/variant selection, plus 24 named user scales. The persistent bank is stored inside the app/extension application-data container. Sandboxed hosts may have separate banks; the active tuning remains embedded in patches and DAW sessions. See USER_MANUAL.md and MAQAM_LIBRARY.md for coverage and approximation limits.
