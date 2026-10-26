# SynthMaven 1.5 — factory levels and instrument appearance

The 537-entry catalogue is retained. **536 audible factory presets are level matched; USER WT stays silent until a user wavetable is uploaded.** This includes the 88 Oriental/Middle Eastern sounds and nine XD compatibility presets.

Factory trims are measured through the actual firmware preset loader and complete stereo mix, using role-appropriate bass, melody, chord and comping phrases. The audit uses gated BS.1770-4 loudness, 4× oversampled peak measurement and a conservative −29 LUFS target. The committed audit records every catalogue entry. Matching applies to those reference phrases at factory settings; note range, velocity, user effects and four-sound layering can change perceived loudness. Previously saved sounds preserve their own settings; reload a factory preset to get its new trim.

The editor adopts the standalone instrument’s rounded panels, system sans-serif font, rotary controls and gridded parameter graphics. Sound, Modulation, Effects, Layers, Tuning and All parameters organize overlapping functions. Envelope handles edit the actual envelope parameters. Other parameter diagrams are previews, not live audio analysis. Existing MIDI editing, preset storage and sequencing remain available.

The editor and installer share the SynthMaven logo’s red, black and white palette. Appearance offers Dark, Light and System, remembered in the browser. Factory browsing and all four program sounds remain accessible on small screens. Installer links open the manual, quick-start guide and release notes.

Bluetooth remains removed. Direct USB MIDI host mode, four-sound programs, optional layering, 46 maqams with selectable root and four custom chromatic scales remain from 1.4. Chocolate Plus hardware and audio-timing validation remain outstanding.

The final host audit measured **−29.304 to −28.744 LUFS**, with the highest 4× true peak at **−3.473 dBTP**. [All 537 entry measurements (CSV)](../assets/factory-level-audit.csv) include the intentionally silent USER WT row. Release CI re-renders every entry and blocks publication if the audible count, ±0.5 LU tolerance or −1 dBTP peak ceiling fails.

The Tuning page includes a chromatic keyboard with the selected root and each key’s cents offset. Altered keys use the logo accent. In CUSTOM1–4, click a key or its note button to cycle 0 / −25 / −50 / −75 cents and save the change on the FM1.

The four-sound program now adapts the standalone layer mixer: enable, independent full-keyboard/split zones, Lower/Upper split side, split point, ±24-semitone transpose with octave buttons, rotary pan, vertical level faders, mute and multiple solo. The hardware provides matching GLO → LAYER ROUTE / MIX / PITCH pages for the HOME-selected slot. Routing edits release the edited slot; level and pan edits keep held notes. PRG2 stores the routing and solo state; existing PRG1 programs upgrade on load. Editor protocol 19 adds command 109; older editors retain their existing commands.
