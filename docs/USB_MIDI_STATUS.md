# USB MIDI keyboard requirement and verification

SynthMaven must accept class-compliant USB MIDI 1.0 keyboard performance data:
notes and note releases, 1–127 note-on velocity (zero means note-off), 14-bit
pitch bend, mod wheel CC1, expression CC11, sustain CC64, breath CC2 when enabled,
channel and polyphonic pressure, engine/controller CCs and MPE pressure/bend/slide.
Sound response depends on the selected patch's modulation assignments. Proprietary
keyboard articulation protocols and automatic commercial-library keyswitch
emulation are not implied by generic MIDI support.

`tests/synthmaven_usb_performance_test.c` exercises the actual USB EP1 packet
parser, ring, sequencer and synth controller/voice state. It checks all 16 MIDI
channels, note-on velocity, zero-velocity release, sustain, bend, expression,
modulation, channel/polyphonic pressure, MPE and safe releases after changing
inputs. The test runs under the firmware suite and passed with ASan/UBSan.

The routing fix makes keyboard performance controls follow KEY IN. Bank Select
and Program Change retain their separate PATCH MIDI routing. Old-source note-off,
sustain-up and safety reset messages remain accepted to avoid hanging notes.

## Physical USB connection status

These tests validate the current USB **device-mode** data path. They are not
physical tests of a directly attached keyboard. Direct keyboard operation requires
FM1 USB host mode, appropriate USB wiring/VBUS arrangements and hardware tests.
A separate keyboard power supply does not implement the USB host software.

An existing local fm1-base BSP checkout contains `chip/usb_host.c`,
`chip/usb_host_desc.h` and `docs/USB_HOST.md`: a candidate host implementation
for class-compliant full-speed MIDI 1.0 devices, with external VBUS power and
one hub tier. Its own documentation marks physical validation pending. It is
not integrated into SynthMaven by this performance-routing change.

Integration must retain computer-facing device mode for editing/update recovery,
release USB-owned voices/pedals on disconnect, preserve TRS/BLE input ownership,
fit the BLE build's tight flash/RAM budgets without removing engines, and test
notes/controllers plus reconnect, host/device switching and audio timing on FM1.
No current direct-host compatibility claim is made.
