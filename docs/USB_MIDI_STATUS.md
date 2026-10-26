# USB MIDI status — 1.4

GLO → USB ROLE selects COMPUTER (USB device) or PEDAL (USB host). PEDAL supports a direct class-compliant full-speed USB MIDI controller, or one USB hub. Use a powered adapter/hub: the implementation does not supply VBUS. It supports USB MIDI 1.0 streaming; HID-only devices and low-speed devices are not supported.

The host driver has deterministic simulation tests for descriptors, direct devices, hubs, hot-plug, reconnect, endpoint errors, ACK/NAK ownership, ring back-pressure and host/device switching. These are software tests. Chocolate Plus direct connection and sustained audio operation require a hardware test.

COMPUTER mode is required for the editor, USB audio and firmware update. Hold OCT− at power-on for device-only recovery. Bluetooth is removed in 1.4.
