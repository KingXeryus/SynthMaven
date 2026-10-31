# USB MIDI controllers and automatic detection

SynthMaven 3.5 development identifies an attached class-compliant USB MIDI controller from its USB vendor identity, product name and MIDI interface name. Automatic is the default; no model selection is required. Unknown devices retain standard MIDI input. Detection does not install vendor drivers or change custom controller templates.

## Connecting

Select USB HOST on the FM-1, then connect the controller through a powered adapter or hub. The editor Settings page provides protocol and virtual-port overrides; the device header shows HOST while hosting. The physical socket cannot simultaneously host a controller and connect to the editor: return to COMPUTER mode before connecting to the computer for editing, backup or installation. Auto-detection does not automatically change this USB role.

## Keyboards, pads and pedals together

USB ROLE describes the electrical connection, not what kind of controller you play:

- **COMPUTER**: connect the FM-1 to a computer or iPad for the editor, USB MIDI and USB audio.
- **HOST**: connect a class-compliant USB MIDI keyboard, pad controller or pedal. There is no separate pedal-only or keys-only USB mode. The FM-1 does not supply VBUS power; a bus-powered controller requires a suitable powered adapter or hub.
- TRS MIDI input remains enabled in either USB role. Connect the controller's MIDI OUT to the FM-1's MIDI IN. Use the controller's standard MIDI port/template rather than its DAW control port for notes.

### Two controllers assigned to different parts

1. Connect one controller through USB (HOST role) and the other through TRS MIDI.
2. Open GLO → SYSTEM and select **ROUT → PARTS**.
3. Set the first controller to MIDI channel 1 and the second to channel 2. Channels 1, 2, 3 and 4 address layers 1, 2, 3 and 4 respectively. Channels outside this range do not play a part in PARTS mode.
4. PARTS keeps external MIDI on the addressed part even when the program's Layering switch is on. The FM-1's own keys retain the program's layered behavior.
5. Assign different channels to independent controllers. Controllers sharing a channel share its bend, sustain and note state; this is not automatic device-per-layer routing.
6. For a MIDI-only forwarding track, **MIDI → BOTH** admits USB and TRS together. Internal synth parts already receive both physical inputs.

For two USB controllers, use a computer/iPad as the host and route both MIDI inputs to the FM-1 in COMPUTER mode. The FM-1 host driver currently services **one USB MIDI device**, directly or behind one hub; it does not service two downstream USB devices simultaneously. A powered hub alone does not supply MIDI routing between devices.

### A pedal alongside a keyboard

1. Connect the pedal to one input and the keyboard to the other, or use distinct MIDI channels if a computer merges them into USB.
2. In the editor, open Settings → MIDI Learn. Learn the pedal message, assign the action and save.
3. On firmware supporting source-scoped assignments, choose **USB only** or **TRS only**. The editor initially selects the connector that supplied the learned message. **Any input** preserves the older behavior.
4. Source-scoped assignments consume matching messages only from that connector. Ordinary keyboard notes need no learned action. Use a dedicated pedal channel to avoid collisions when pedal and keyboard share a connector.
5. Duplicate suppression remains per assignment; a released pedal rearms an intentional repeated tap.

### Connection checks

The SYSTEM MIDI status shows recent RX activity. With USB selected in HOST mode it shows HOST after a MIDI interface has been configured, or WAIT while waiting for one. This status no longer reports OFF merely because the computer-facing USB interface is inactive.

The shared header shows WAIT until the MIDI interface is configured, then HOST; it does not depend on space remaining in the MIDI knob card.

### Hubs

- The controller may sit behind one hub, or behind a hub on that hub's port: many multi-port hubs are two hub chips in one case, with most sockets on the second chip. A third hub level is not searched.
- Some hubs answer only after several hundred milliseconds following a bus reset. When nothing answers, the FM-1 resets again with longer recovery waits (100, 200, 400, 800 and 1600 ms), so finding a slow hub can take about 3 seconds. A device that answered is not reset again.
- These host-enumeration changes follow Melodee upstream (commits 7eb442e and 6e61576). The regression model covers direct MIDI devices, one hub, two-chip hubs, slow hubs, disconnect/reconnect, endpoint stalls and back-pressure.

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

Named USB MIDI jacks select the actual control cable where available; family profiles supply a fallback when jack names are absent. MiniLab 3 uses the third USB cable for MCU/HUI, keeping its separate DIN Thru cable musical. Mackie manufacturer handshakes and HUI traffic confirm the protocol. The editor provides an advanced protocol/port override for unusual USB layouts. Standard MIDI override disables surface decoding. Settings preserve the override across restarts.

After using Launchpad programmer mode, use the controller's mode control or restart it to return to its usual standalone mode. APC Mini's separate Note Mode port keeps its original note mapping.

## Validation and limits

Automatic identity rules, USB cable separation, MCU/HUI decoding, handshake replies, and pad mappings are covered by host-side tests. USB enumeration and error handling are tested against a simulated device model. The development candidate received a channel-2 note and release from the Akai MPK mini IV DIN output through a Type-A TRS adapter while the FM-1 remained connected to the Mac by USB. In 3.5.3 the Akai MPK mini IV was confirmed playing through a powered multi-port USB hub in HOST role. Other hubs and controllers are untested; a powered connection and class-compliant USB MIDI interface are required. Proprietary drivers and arbitrary composite USB layouts are not guaranteed.

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

## Routing layers and the drummer

Open Settings → MIDI → **Routing** in the web editor and turn on **Use custom routing**. Each of layers 1–4 and the Drummer gets an input (Any, USB or TRS) and a channel (Off, 1–16 or All channels). Routing replaces SYSTEM → ROUT while it is on, and is saved on the FM-1, so it applies in COMPUTER and USB HOST.

- **Splits and stacks:** give two layers the same input and channel, then set their note ranges. Separate ranges split the keyboard; overlapping ranges stack. Ranges are saved with the program.
- **Drummer:** plays and records on the first part using the DRUM engine, so that part is not available as a layer. Pads use the General MIDI drum map. A layer may not share an input and channel with the Drummer.
- **Recording:** MIDI Learn offers Transport Play / Stop, Record (selected part) and Record (drummer). One part records at a time.
- **Forwarding:** in COMPUTER mode, the web editor's Connection & forwarding tab sends each controller on the computer to a target on that target's channel. It stops when the editor closes. In USB HOST there is no forwarding and one USB controller is served, so give its target All channels or send the assigned channel.
- **Going back:** older firmware ignores the routing record, and a transport assignment acts as All notes off there.
