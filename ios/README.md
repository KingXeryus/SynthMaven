# SynthMaven Editor for iPhone and iPad

Native iOS app wrapping the **existing SynthMaven web editor**, bundled offline. Version 3.0 accompanies SynthMaven firmware 3.0. Version 3.0 (8), displayed as **SM FM1 Editor** on the Home Screen, is available to the internal FM1 Release Testing group and submitted for external FM1 Friends testing. External availability requires Apple's Beta App Review approval. It is not a public App Store release. The next app release is prepared as **3.1 (9)** and retains the exact spaced Home Screen name **SM FM1 Editor**; that next build is not yet uploaded.

## USB editing

1. Set the FM-1 to **COMPUTER** USB mode.
2. Connect it to the iPhone/iPad with a USB data cable and a suitable powered adapter/hub if required. Lightning devices require an appropriate USB adapter.
If only Network Session or another instrument appears, iPadOS has not enumerated the FM-1. Reconnect in COMPUTER mode; try the known-working data cable with a USB adapter or powered hub instead of a direct USB-C cable. The device may appear as **Melodee FM-1** because of its USB identity.

3. Tap **Connect**, choose **USB**, and select the detected FM-1. The connection wizard matches its input/output and starts the editor handshake.
4. If automatic matching is unavailable, use MIDI ports to select the FM-1 input and matching output.
5. Edit sounds, programs, layers and tuning using the existing pages. Saving uses the existing editor actions; opening the app does not reset the FM-1.

Exports use the iOS share sheet (choose Save to Files). Imports use the existing file pickers. The library uses the app's persistent WebKit storage. Removing the app removes its local library; export a backup first. Installation/recovery is not included in this app.

## Bluetooth compatibility

Firmware 3.0 excludes the FM-1 radio. Connect the FM-1 by USB in COMPUTER mode. The app retains Core MIDI Bluetooth pairing for other compatible devices and external bidirectional SysEx MIDI bridges; this does not enable Bluetooth inside the FM-1. Direct radio support remains research-only because dual-core hardware testing exposed a fault.

## Audio-load controls and limits

- Existing serial request/reply protocol and latest-value control coalescing are retained.
- Native output is bounded to at most 25 messages/second for USB and 12.5 for Bluetooth, with additional byte-based pacing. Queue capacity is 32; overflow reports a disconnect instead of losing writes silently.
- Replies are reconstructed across Core MIDI packet boundaries; realtime MIDI bytes do not corrupt fragmented SysEx.
- Disconnect, unplug and backgrounding discard queued writes and invalidate session IDs. Reconnect requires an explicit new port selection and editor connection.
- No background app scans, DSP changes or automatic gain adjustments.

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

The shared scheme is committed. `generate_project.py` regenerates the project if source membership changes. `prepare_editor.py` copies the production interface and assets; generated content is ignored by Git. Its transport override is a 10-second minimum reply timeout for native transports; installer/documentation links open the public website. It does not modify the desktop editor.

## Validation

The automated suite tests native SysEx fragmentation, realtime/running status, queue bounds, USB/Bluetooth pacing, disconnect handling, the JavaScript bridge and the iOS simulator build. Firmware transport tests cover full-size fragmented requests and replies. See the 3.0 release notes for actual device timing results and remaining hardware limitations. A simulator build is not App Store approval or physical iOS end-to-end certification.

## Licensing and boundaries

GPL-3.0-only, consistent with the existing editor. Bundled font/logo notices are retained. Only the bundled main editor page can call the MIDI/file bridge. External links open outside the privileged WebView. The native sender accepts the editor's SysEx envelope and channel MIDI, not firmware-updater traffic. MIDI transport runs outside audio callbacks; this app does not synthesize audio.

The editor and its native connection dialogs always use dark mode. There is no light-mode selector.

## App identity

The iPad and iPhone Home Screen name is **SM FM1 Editor**. The app icon uses the supplied SynthMaven FM1 artwork in `Branding/fm1-app-icon.svg`; the editor uses `Branding/in-app-logo.svg`. The 1024-pixel opaque icon is bundled through `Assets.xcassets/AppIcon.appiconset`. Regenerating the Xcode project preserves the icon and display name.

Perform quick controls apply to the selected part: four existing macros, Tone,
Distortion, Delay and Reverb, plus LFO rate/filter depth and envelope attack/release.
Tone follows the engine's exposed cutoff (HYBRID uses its cutoff offset); engines
without that control display Unavailable. Delay and Reverb are per-part sends.
These controls reuse the existing MIDI SET queue and saved sound parameters;
opening Perform does not change sound settings or add audio processing.

The app opens directly in Perform with all twelve quick controls visible. Before
connection they are disabled and read Connect FM-1; after connection they reflect
the selected part. In Edit mode, Load Vital, Load DX7 and Load Minilogue XD open the existing
preset importers in a dialog. These buttons are hidden in Perform. Imports retain their preview,
compatibility information and explicit send actions. DX7 uses FM6; Minilogue XD
uses HYBRID. The loader offers an explicit engine switch rather than replacing a
sound just because the dialog opened. Done returns to Edit.

Edit includes Assign Macros 1–4. Choose a destination and signed modulation amount,
then Add route or Apply. A macro can control multiple destinations; unrelated matrix
routes and existing HYBRID VIA sources are preserved. A full matrix reports an error
rather than overwriting another modulation. Non-HYBRID macros retain their native
brightness/timbre/motion/space behavior in addition to assigned routes.
Save sound to library captures the selected sound including its HYBRID patch data;
Save program opens the existing named program slots and saves all four parts.
Assignments are not permanent until saved. The duplicate inline Vital importer and
Sound Tools are hidden in iOS; the Edit Vital loader remains available.

Perform → Layers uses four compact mixer strips with level faders, pan, enable,
mute and solo. Tap the routing disclosure for split mode, zone, split note,
transpose and master-tuning inclusion. The top part selectors remain available.
Perform → Tuning shows the master scale, root, octave threshold, A4 reference,
Scala import/send and the chromatic tuning keyboard. White/black keys retain their
normal colors; detuned keys are red with cent labels. Tap a key once to select it without changing its tuning. The horizontal wheel
sets its offset in 1-cent increments (left lowers, right raises), or enter an exact
value from −100 to +100 cents. Double-tap an untuned key for −50 cents; double-tap
a tuned key to reset it to zero. Quick microtuning places the borderless wheel at the left, bottom-aligned with the keyboard. The full Tuning page centers it below the keyboard. Closing Quick microtuning expands the Perform controls into the released space. Fine edits require firmware protocol 26. When
starting from a factory scale, choose the Custom 1–4 destination to replace. This is a tuning display, not a playing keyboard.
