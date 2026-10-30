// SPDX-License-Identifier: GPL-3.0-only
import Foundation

/// Core MIDI may split SysEx across packets and interleave realtime bytes.
struct MIDIStream {
    var bytes: [UInt8] = []
    var running: UInt8 = 0
    var sysex = false
    mutating func feed(_ data: [UInt8]) -> [[UInt8]] {
        var messages: [[UInt8]] = []
        for byte in data {
            if byte >= 0xF8 { messages.append([byte]); continue }
            if byte == 0xF0 { bytes = [byte]; sysex = true; running = 0; continue }
            if sysex {
                if byte == 0xF7 { bytes.append(byte); messages.append(bytes); bytes = []; sysex = false }
                else if byte & 0x80 != 0 { bytes = []; sysex = false }
                else if bytes.count < 65536 { bytes.append(byte) }
                else { bytes = []; sysex = false }
                continue
            }
            if byte & 0x80 != 0 {
                bytes = [byte]; running = byte < 0xF0 ? byte : 0
                if length(byte) == 1 { messages.append(bytes); bytes = [] }
            } else {
                if bytes.isEmpty && running != 0 { bytes = [running] }
                guard !bytes.isEmpty else { continue }
                bytes.append(byte)
                if bytes.count == length(bytes[0]) { messages.append(bytes); bytes = [] }
            }
        }
        return messages
    }
    private func length(_ status: UInt8) -> Int {
        if status < 0xF0 { return status & 0xE0 == 0xC0 ? 2 : 3 }
        return status == 0xF2 ? 3 : (status == 0xF1 || status == 0xF3 ? 2 : 1)
    }
}

/// One complete frame at a time. Overflow is an error, never silent data loss.
struct MIDIQueue {
    var pending: [[UInt8]] = []
    var nextSend: TimeInterval = 0
    var interval: TimeInterval = 0.04
    mutating func append(_ bytes: [UInt8]) throws {
        guard !bytes.isEmpty, bytes.count <= 8192, pending.count < 32 else { throw MIDIProblem.invalid("MIDI queue full or message too large") }
        if bytes[0] == 0xF0 {
            // This app is an editor, not a firmware updater or arbitrary SysEx sender.
            guard bytes.count >= 6, Array(bytes.prefix(4)) == [0xF0,125,70,76], bytes.last == 0xF7,
                  bytes.dropFirst().dropLast().allSatisfy({ $0 < 128 }) else { throw MIDIProblem.invalid("Unsupported SysEx") }
        } else {
            guard bytes[0] >= 0x80, bytes[0] < 0xF0, bytes.count == ((bytes[0] & 0xE0 == 0xC0) ? 2 : 3),
                  bytes.dropFirst().allSatisfy({ $0 < 128 }) else { throw MIDIProblem.invalid("Unsupported MIDI message") }
        }
        pending.append(bytes)
    }
    mutating func take(now: TimeInterval) -> [UInt8]? {
        guard now >= nextSend, !pending.isEmpty else { return nil }
        let bytes = pending.removeFirst()
        nextSend = now + max(interval, Double(bytes.count) / 4096)
        return bytes
    }
    mutating func clear() { pending.removeAll(); nextSend = 0 }
}
enum MIDIProblem: Error, LocalizedError {
    case invalid(String)
    var errorDescription: String? { if case .invalid(let text) = self { return text }; return nil }
}

// CoreMIDI can retain an older firmware's user-visible device name indefinitely.
// Confirm renamed USB candidates by transport and VID/PID; the editor still
// validates SynthMaven's protocol when connecting before reading or editing.
func isFM1MIDIIdentity(names: [String], driver: String, usbVendorProduct: Int32) -> Bool {
    let known = names.contains { name in
        let n = name.lowercased()
        return n.contains("synthmaven") || n.contains("fm-1") || n.contains("melodee") || n.contains("felucca")
    }
    return known || (driver.lowercased().contains("usb") && usbVendorProduct == 0x12090001)
}
