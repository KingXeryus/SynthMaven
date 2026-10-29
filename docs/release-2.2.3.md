# SynthMaven 2.2.3

Solo sounds retain their calibrated levels and stored solo compensation. Factory programs use three lightweight mono backing parts with one oscillator and no drive, while the main part keeps the instrument focus. P5/P6/P7 simplify duplicated oscillator registrations; the original standalone presets remain available. The previous 12 dB layered attenuation is removed. Layered mixes retain sample look-ahead limiting. Autosave waits for quiet audio before copying program state, and simultaneous mono backing-note restarts are combined within one audio block. The eight-voice budget is shared across all parts.

Hardware tests before the final note-batching change showed 64–75% average CPU peaks across the eight factory programs. Regular one- through five-note patterns had no missed deadlines; dense eight-note bursts still produced one missed deadline in P5 and P7. The final batching correction passes host tests but has not been retested on hardware, at the user's request. Custom heavy layers can still overload the processor; host renders do not establish hardware timing compliance.

Installation resets settings, programs, user presets, samples and autosave, including same-version reinstalls. Back up first. The corrected loader successfully restarted the connected FM-1 automatically during candidate installs.

The loader uses a shared hardware flash-range guard and services the watchdog between reset erases. Regression coverage includes slow erases, reset failures, recovery retries, corrupt updates and same-version reinstalls. Audio audits cover clipping, true peaks, solo and layered playback, scenes and mono compatibility.

Use the [installer](https://kingxeryus.github.io/SynthMaven/webapp/installer/), connect by USB in COMPUTER mode, back up in the editor, then install without unplugging. If interrupted, reconnect in the device recovery loader and retry installation. The PDF guide and INSTALL-AND-RECOVERY.md provide the complete procedure. Bluetooth is not supported. Direct USB host mode requires a powered adapter or hub; controller compatibility remains device-dependent.
