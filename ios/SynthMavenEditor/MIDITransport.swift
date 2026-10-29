// SPDX-License-Identifier: GPL-3.0-only
import Foundation
import CoreMIDI

struct MIDIPortChoice {
    let endpoint: MIDIEndpointRef
    let name: String
    let transport: String
}
final class MIDITransport {
    var onMessage: (([UInt8]) -> Void)?
    var onChange: (() -> Void)?
    var onError: ((String) -> Void)?
    private(set) var source: MIDIEndpointRef = 0
    private(set) var destination: MIDIEndpointRef = 0
    private(set) var label = "Not connected"
    private var client = MIDIClientRef()
    private var input = MIDIPortRef()
    private var output = MIDIPortRef()
    private let receiveQueue = DispatchQueue(label: "SynthMaven.MIDI.receive")
    private var parser = MIDIStream()
    private var scheduler = MIDIQueue()
    private var timer: Timer?
    private var generation: UInt = 0
    var sessionID: UInt { generation }
    private var packetStorage: UnsafeMutableRawPointer

    init() {
        packetStorage = .allocate(byteCount: 16384, alignment: 16)
        let status = MIDIClientCreateWithBlock("SynthMaven Editor" as CFString, &client) { [weak self] _ in
            DispatchQueue.main.async { self?.refresh() }
        }
        guard status == noErr else { return }
        MIDIInputPortCreateWithBlock(client, "Editor input" as CFString, &input) { [weak self] list, context in
            guard let self else { return }
            let session = UInt(bitPattern: context)
            var data: [UInt8] = []
            for packet in list.unsafeSequence() { data.append(contentsOf: packet.bytes()) }
            self.receiveQueue.async {
                // Parser belongs to this serial queue. Discard old-session fragments.
                guard session == self.receiveGeneration else { return }
                let messages = self.parser.feed(data)
                DispatchQueue.main.async {
                    guard session == self.generation, self.source != 0 else { return }
                    for message in messages { self.onMessage?(message) }
                }
            }
        }
        MIDIOutputPortCreate(client, "Editor output" as CFString, &output)
        timer = Timer.scheduledTimer(withTimeInterval: 0.01, repeats: true) { [weak self] _ in self?.flush() }
    }
    private var receiveGeneration: UInt = 0
    var available: Bool { client != 0 && input != 0 && output != 0 }
    func ports(input: Bool) -> [MIDIPortChoice] {
        let count = input ? MIDIGetNumberOfSources() : MIDIGetNumberOfDestinations()
        return (0..<count).compactMap { index in
            let endpoint = input ? MIDIGetSource(index) : MIDIGetDestination(index)
            guard endpoint != 0 else { return nil }
            return MIDIPortChoice(endpoint: endpoint, name: property(endpoint, kMIDIPropertyDisplayName), transport: property(endpoint, kMIDIPropertyDriverOwner))
        }
    }
    private func property(_ object: MIDIObjectRef, _ key: CFString) -> String {
        var value: Unmanaged<CFString>?
        guard MIDIObjectGetStringProperty(object, key, &value) == noErr, let value else { return "MIDI device" }
        return value.takeRetainedValue() as String
    }
    func connect(source: MIDIPortChoice, destination: MIDIPortChoice, wireless: Bool) throws {
        disconnect()
        guard available else { throw MIDIProblem.invalid("Core MIDI is unavailable") }
        generation &+= 1
        receiveQueue.sync { parser = MIDIStream(); receiveGeneration = generation }
        let result = MIDIPortConnectSource(input, source.endpoint, UnsafeMutableRawPointer(bitPattern: generation))
        guard result == noErr else { throw MIDIProblem.invalid("Could not open MIDI input (\(result))") }
        self.source = source.endpoint; self.destination = destination.endpoint
        scheduler.interval = wireless ? 0.08 : 0.04
        label = "\(wireless ? "Bluetooth" : "USB") · \(source.name)"
        onChange?()
    }
    func send(_ bytes: [UInt8]) throws {
        guard source != 0, destination != 0 else { throw MIDIProblem.invalid("MIDI disconnected") }
        try scheduler.append(bytes)
    }
    private func flush() {
        guard destination != 0, let bytes = scheduler.take(now: ProcessInfo.processInfo.systemUptime) else { return }
        let list = packetStorage.bindMemory(to: MIDIPacketList.self, capacity: 1)
        let first = MIDIPacketListInit(list)
        _ = bytes.withUnsafeBufferPointer { buffer in MIDIPacketListAdd(list, 16384, first, 0, buffer.count, buffer.baseAddress!) }
        let result = MIDISend(output, destination, list)
        if result != noErr { fail("MIDI send failed (\(result))") }
    }
    private func fail(_ text: String) { disconnect(); onError?(text) }
    func disconnect() {
        generation &+= 1
        if source != 0 { MIDIPortDisconnectSource(input, source) }
        source = 0; destination = 0; scheduler.clear(); label = "Not connected"
        receiveQueue.sync { parser = MIDIStream(); receiveGeneration = generation }
        onChange?()
    }
    func refresh() {
        if source != 0 && (!ports(input: true).contains(where: {$0.endpoint == source}) || !ports(input: false).contains(where: {$0.endpoint == destination})) { disconnect() }
        onChange?()
    }
    deinit {
        timer?.invalidate()
        if source != 0 { MIDIPortDisconnectSource(input, source) }
        if input != 0 { MIDIPortDispose(input) }; if output != 0 { MIDIPortDispose(output) }
        if client != 0 { MIDIClientDispose(client) }; packetStorage.deallocate()
    }
}
