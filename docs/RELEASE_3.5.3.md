# SynthMaven 3.5.3 — USB MIDI host through powered hubs

USB keyboards, pads and pedals now work through a powered USB hub in **HOST** role. Before this release the FM-1 gave up on the hub before it had finished starting, so the controller was never reached.

## USB host

- **Slow hubs:** a root reset now lasts 50 ms, and a hub that does not answer is reset again with longer recovery waits (100 ms up to 1.6 s). Finding a slow hub can take about 3 seconds. A device that answered is not reset again.
- **Two-chip hubs:** many multi-port hubs contain two hub chips, with most sockets on the second. The controller is now found behind that second chip. A third hub level is not searched.
- These changes follow Melodee upstream (7eb442e, 6e61576). Earlier experimental USB register changes that did not help were removed.
- The menu and editor call the role **HOST** (previously PEDAL). It is for keyboards, pads and pedals; there is no separate keys-only or pedal-only mode.
- The FM-1 cannot power its USB port. A bus-powered controller needs a powered hub or powered OTG adapter; a direct cable to the controller does not work.

## Keyboards, pads and pedals together

- TRS MIDI IN stays active in either USB role.
- GLO → MIDI/CPU → **ROUT = PARTS** sends MIDI channels 1–4 to layers 1–4, even with Layering on, so a USB controller and a TRS controller on different channels can play different layers.
- **MIDI = BOTH** lets a MIDI-only forwarding track accept USB and TRS together.
- MIDI Learn assignments can be limited to **USB only** or **TRS only**, so a pedal on one connector and a keyboard on the other do not interfere. Existing assignments keep **Any input**.

## Installation

1. Export a complete backup first: installation resets settings and musical storage to factory defaults.
2. Set USB ROLE to COMPUTER and install with the web installer and a data cable. Restore your backup afterwards.

## Validation and limitations

Confirmed by the owner on hardware: an Akai MPK mini IV plays the FM-1 through a powered multi-port USB-C hub (Lemorele) in HOST role, and TRS input works while USB is connected. Host-side regression tests cover direct devices, one hub, two-chip hubs, slow hubs, disconnect/reconnect, stalls and back-pressure; stack-budget and target memory checks pass. Other hubs and controllers are untested. The host driver plays one USB MIDI controller at a time. Bluetooth remains excluded.

The illustrated PDF guide is updated for 3.5.3. Release firmware is built and attested by the required shared builder.
