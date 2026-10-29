# USB MIDI controllers and automatic detection

SynthMaven 2.2 identifies an attached class-compliant USB MIDI controller from its USB vendor identity, product name and MIDI interface name. Automatic is the default; no model selection is required. Unknown devices retain standard MIDI input. Detection does not install vendor drivers or change custom controller templates.

## Connecting

Select USB PEDAL/HOST on the FM-1, then connect the controller through a powered adapter or hub. The GLO CONTROLLER page shows the detected product, family and active protocol. The physical socket cannot simultaneously host a controller and connect to the editor: return to COMPUTER mode before connecting to the computer for editing, backup or installation. Auto-detection does not automatically change this USB role.

## Profile coverage

| Manufacturer | Recognized families and examples | Mapping |
|---|---|---|
| Alesis | V25/V49/V61, VI25/VI49/VI61, V Mini | Standard notes, velocity and MIDI controllers |
| Novation | Launchkey, FLkey, Impulse | Standard music port; MCU/HUI on the separate DAW port |
| Novation | Launchpad X, Mini MK3, Pro MK3 | Automatic programmer mode, chromatic 8×8 pads C2–D♯7 and pad feedback |
| Arturia | MiniLab, KeyLab, KeyStep, BeatStep, MicroLab | Standard MIDI; MiniLab/KeyLab separate DAW-port support |
| Akai | MPK, MPD, LPD8, LPK25, MIDImix, APC | Standard MIDI; APC Mini has chromatic grid mapping and first four faders mapped to part volumes |
| Mackie | MCU Pro / Mackie Control, HUI | Four program-part channel strips and transport |

The catalogue identifies families, not a claim that every generation or every custom preset has been physically tested. Standard MIDI profiles use the existing note/CC behavior; they do not reproduce manufacturer DAW scripts. MIDImix and non-Mini APC controls require normal MIDI assignments. Controller DAW mode may need to be enabled on the controller itself. A generic MIDI adapter cannot expose the identity of a DIN controller behind it.

## MCU and HUI controls

The first four strips control the four program parts: volume faders, pan encoders, mute, solo, select and record arm. Play, stop and record control the sequencer. Fader, pan, switch and transport feedback are sent back; touched motor faders are not overwritten. Additional strips, master fader, scribble strips and DAW automation are not implemented. Musical pitch bend on a keyboard's music port remains pitch bend, rather than becoming a mixer fader.

Named USB MIDI jacks select the actual control cable where available; family profiles supply a fallback when jack names are absent. MiniLab 3 uses the third USB cable for MCU/HUI, keeping its separate DIN Thru cable musical. Mackie manufacturer handshakes and HUI traffic confirm the protocol. GLO CONTROLLER and the editor provide an advanced protocol/port override for unusual USB layouts. Standard MIDI override disables surface decoding. Settings preserve the override across restarts.

Launchpad programmer mode is restored to Live/standalone mode when returning to COMPUTER mode while still connected. An abrupt unplug cannot send that restoration; use the controller's mode control or restart it. APC Mini's separate Note Mode port keeps its original note mapping.

## Validation and limits

Automatic identity rules, USB cable separation, MCU/HUI decoding, handshake replies, and pad mappings are covered by host-side tests. USB enumeration and error handling are tested against a simulated device model. Physical FM-1/controller validation remains pending; a powered connection and class-compliant USB MIDI interface are required. Proprietary drivers and arbitrary composite USB layouts are not guaranteed.

## Sources

- [Alesis controller range](https://www.alesis.com/products/browse/category/midi-controllers.html)
- [Novation Launchkey](https://us.novationmusic.com/launchkey)
- [MiniLab 3 port roles](https://support.arturia.com/hc/en-us/articles/6189475866396-MiniLab-3-General-Questions)
- [Arturia controllers](https://www.arturia.com/store/midi-controllers-sequencers)
- [Akai controllers](https://www.akaipro.com/products/controllers/)
- [Mackie MCU Pro manual](https://mackie.com/img/file_resources/MCU_Pro-XT_Pro_OM.pdf)
- [MCU protocol reference by TouchMCU's author](https://github.com/NicoG60/TouchMCU/blob/main/doc/mackie_control_protocol.md)
- [HUI receive, transmit and zone reference](https://github.com/anselbrandt/midi-hui)
- [APC Mini MK2 communication protocol](https://cdn.inmusicbrands.com/akai/attachments/APC%20mini%20mk2%20-%20Communication%20Protocol%20-%20v1.0.pdf)
- [Launchpad X programmer reference](https://fael-downloads-prod.focusrite.com/customer/prod/s3fs-public/downloads/Launchpad%20X%20-%20Programmers%20Reference%20Manual.pdf)
- [Launchpad Pro MK3 programmer reference](https://fael-downloads-prod.focusrite.com/customer/prod/s3fs-public/downloads/LPP3_prog_ref_guide_200415.pdf)

The machine-readable family catalogue is `assets/controllers/catalog.json`; `tools/gen_controllers.py` generates the bounded firmware identity rules.
