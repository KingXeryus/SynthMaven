# SynthMaven Editor for iPhone and iPad

Native iOS app wrapping the **existing SynthMaven web editor**, bundled offline. Firmware remains 2.2.4; the app has its own development version, 0.1.0. This is a development build, not an App Store/TestFlight release.

## USB editing

1. Set the FM-1 to **COMPUTER** USB mode.
2. Connect it to the iPhone/iPad with a USB data cable and a suitable powered adapter/hub if required. Lightning devices require an appropriate USB adapter.
3. Open **MIDI ports** in the app, select the FM-1 input and matching output.
4. Tap **Connect** in the existing editor. It checks the device identity and reads its controls.
5. Edit sounds, programs, layers and tuning using the existing pages. Saving uses the existing editor actions; opening the app does not reset the FM-1.

Exports use the iOS share sheet (choose Save to Files). Imports use the existing file pickers. The library uses the app's persistent WebKit storage. Removing the app removes its local library; export a backup first. Installation/recovery is not included in this app.

## Bluetooth editing

**Bluetooth pairing and native MIDI transport are implemented in the app. Direct Bluetooth editing with FM-1 firmware 2.2.4 is not available.** That firmware intentionally has Bluetooth disabled.

Use a bidirectional, SysEx-capable BLE MIDI bridge connected to the FM-1 USB port, with the bridge acting as USB host and the FM-1 in COMPUTER mode. No specific adapter has yet been validated. A Bluetooth pedal is not a substitute for this bridge.

1. Open **Bluetooth → Pair MIDI device** and connect the bridge in Apple's MIDI pairing screen.
2. Choose **Done**, then select its MIDI input and matching output.
3. Tap **Connect** in the editor. Pairing alone does not prove SysEx or FM-1 compatibility.

Future direct FM-1 BLE editing needs separate firmware work and actual radio/audio deadline testing. This app does not enable the FM-1 radio or silently install experimental firmware. The external bridge approach keeps Bluetooth processing off the FM-1.

## Audio-load controls and limits

- Existing serial request/reply protocol and latest-value control coalescing are retained.
- Native output is bounded to at most 25 messages/second for USB and 12.5 for Bluetooth, with additional byte-based pacing. Queue capacity is 32; overflow reports a disconnect instead of losing writes silently.
- Replies are reconstructed across Core MIDI packet boundaries; realtime MIDI bytes do not corrupt fragmented SysEx.
- Disconnect, unplug and backgrounding discard queued writes and invalidate session IDs. Reconnect requires an explicit new port selection and editor connection.
- No background scans, firmware radio changes, DSP changes or automatic gain adjustments.

These limits reduce editing traffic; they **do not guarantee** deadline compliance for every layered program or make flash saves non-disruptive. Bulk imports, backups and saves should be done between performances. Device deadline/CPU testing during rapid edits remains required before claiming performance certification.

## Build

Requires Xcode with an iOS SDK (iOS 16+ target), Python 3, and no third-party Swift dependencies.

```sh
python3 ios/prepare_editor.py
xcodebuild -project ios/SynthMavenEditor.xcodeproj -scheme SynthMavenEditor \
  -destination 'generic/platform=iOS Simulator' -derivedDataPath build/ios \
  CODE_SIGNING_ALLOWED=NO build
```

Open `ios/SynthMavenEditor.xcodeproj` in Xcode. Select your Apple development team under Signing & Capabilities, connect your iPhone/iPad, and Run. A physical-device installation requires signing; the unsigned build cannot be installed as an ordinary downloaded app. TestFlight requires an Apple Developer account, signing and App Store Connect setup. No account credentials or signing keys are stored here.

The shared scheme is committed. `generate_project.py` regenerates the project if source membership changes. `prepare_editor.py` copies the production interface and assets; generated content is ignored by Git. Its transport override is a 1.5-second minimum reply timeout for native transports; installer/documentation links open the public website. It does not modify the desktop editor.

## Validation completed

- Simulator and iOS-device builds; development signing succeeded using the existing local Apple team.
- Development build installed on the paired iPad as **SynthMaven Editor**. This is not a TestFlight/App Store distribution.
- Native SysEx fragmentation/realtime/running-status tests.
- USB/Bluetooth scheduler limits, ordering, bounds and queue clearing.
- JavaScript bridge connection, native framing, disconnect and stale-port rejection.
- A read-only INFO request using this native Core MIDI transport received `FELUCCA SYNTHMAVEN 2.2.3` from the connected FM-1 on the development Mac. No device data was changed.

Still required: signed physical iPhone/iPad end-to-end editing, Files export/import on device, compatible Bluetooth bridge testing, and CPU/deadline monitoring while editing layered programs. A successful Mac transport test is not an iOS hardware test.

## Licensing and boundaries

GPL-3.0-only, consistent with the existing editor. Bundled font/logo notices are retained. Only the bundled main editor page can call the MIDI/file bridge. External links open outside the privileged WebView. The native sender accepts the editor's SysEx envelope and channel MIDI, not firmware-updater traffic. MIDI transport runs outside audio callbacks; this app does not synthesize audio.
