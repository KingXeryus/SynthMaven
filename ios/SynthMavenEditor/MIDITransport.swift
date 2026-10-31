// SPDX-License-Identifier: GPL-3.0-only
import Foundation
import CoreMIDI

struct MIDIPortChoice {
    let endpoint: MIDIEndpointRef
    let name: String
    let transport: String
    let entity: MIDIEntityRef
    let isFM1: Bool
    var displayName: String { isFM1 && !name.lowercased().contains("synthmaven") ? "SynthMaven FM1 · \(name)" : name }
}
final class MIDITransport {
    private(set) var initializationError: String?
    var onMessage: (([UInt8]) -> Void)?
    var onChange: (() -> Void)?
    var onError: ((String) -> Void)?
    private(set) var source: MIDIEndpointRef = 0
    private(set) var destination: MIDIEndpointRef = 0
    private(set) var label = "Not connected"
    private var client = MIDIClientRef()
    private var input = MIDIPortRef()
    private var output = MIDIPortRef()
    private var controllerInput = MIDIPortRef()
    private(set) var controllerSource: MIDIEndpointRef = 0
    private(set) var controllerName = "None"
    private var controllerGeneration: UInt = 0
    private var controllerParser = MIDIStream()
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
        guard status == noErr else { initializationError = "Could not start the iPad MIDI service (\(status)). Restart SM FM1; if this persists, restart the iPad."; return }
        let inputStatus = MIDIInputPortCreateWithBlock(client, "Editor input" as CFString, &input) { [weak self] list, context in
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
        guard inputStatus == noErr else { initializationError = "Could not create MIDI input (\(inputStatus))."; return }
        MIDIInputPortCreateWithBlock(client, "Controller input" as CFString, &controllerInput) { [weak self] list, context in
            let token = UInt(bitPattern: context)
            var data: [UInt8] = []
            for packet in list.unsafeSequence() { data.append(contentsOf: packet.bytes()) }
            DispatchQueue.main.async {
                guard let self, token == self.controllerGeneration, self.controllerSource != 0 else { return }
                for message in self.controllerParser.feed(data) where message.first.map({ $0 >= 0x80 && $0 < 0xF0 }) == true {
                    self.sendPacket(message)
                }
            }
        }
        let outputStatus = MIDIOutputPortCreate(client, "Editor output" as CFString, &output)
        guard outputStatus == noErr else { initializationError = "Could not create MIDI output (\(outputStatus))."; return }
        timer = Timer.scheduledTimer(withTimeInterval: 0.01, repeats: true) { [weak self] _ in self?.flush() }
        #if DEBUG
        refresh()
        #endif
    }
    private var receiveGeneration: UInt = 0
    var available: Bool { client != 0 && input != 0 && output != 0 }
    func ports(input: Bool) -> [MIDIPortChoice] {
        let count = input ? MIDIGetNumberOfSources() : MIDIGetNumberOfDestinations()
        return (0..<count).compactMap { index in
            let endpoint = input ? MIDIGetSource(index) : MIDIGetDestination(index)
            guard endpoint != 0 else { return nil }
            let name = property(endpoint,kMIDIPropertyDisplayName), driver = property(endpoint,kMIDIPropertyDriverOwner)
            var entity=MIDIEntityRef(), device=MIDIDeviceRef(), vendorProduct:Int32=0
            MIDIEndpointGetEntity(endpoint,&entity)
            if entity != 0 { MIDIEntityGetDevice(entity,&device) }
            if device != 0 { MIDIObjectGetIntegerProperty(device,"USBVendorProduct" as CFString,&vendorProduct) }
            let names = [name, property(device,kMIDIPropertyName), property(device,kMIDIPropertyModel)]
            return MIDIPortChoice(endpoint:endpoint,name:name,transport:driver,entity:entity,
                isFM1:isFM1MIDIIdentity(names:names,driver:driver,usbVendorProduct:vendorProduct))
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
        sendPacket(bytes)
    }
    private func sendPacket(_ bytes: [UInt8]) {
        guard destination != 0 else { return }
        let list = packetStorage.bindMemory(to: MIDIPacketList.self, capacity: 1)
        let first = MIDIPacketListInit(list)
        let added = bytes.withUnsafeBufferPointer { buffer in MIDIPacketListAdd(list, 16384, first, 0, buffer.count, buffer.baseAddress!) }
        guard added != nil else { fail("MIDI frame exceeds packet capacity"); return }
        let result = MIDISend(output, destination, list)
        if result != noErr { fail("MIDI send failed (\(result))") }
    }
    private func fail(_ text: String) { disconnect(); onError?(text) }
    func routeController(_ port: MIDIPortChoice?) throws {
        if let port, port.endpoint == source { throw MIDIProblem.invalid("Choose the controller, not the SynthMaven FM1 input") }
        guard port == nil || source != 0 else { throw MIDIProblem.invalid("Connect the SynthMaven FM1 first") }
        stopController()
        if let port {
            let result = MIDIPortConnectSource(controllerInput, port.endpoint, UnsafeMutableRawPointer(bitPattern: controllerGeneration))
            guard result == noErr else { throw MIDIProblem.invalid("Could not open controller (\(result))") }
            controllerSource = port.endpoint; controllerName = port.name
        }
        onChange?()
    }
    private func stopController() {
        controllerGeneration &+= 1
        let previous = controllerSource
        controllerSource = 0
        if previous != 0 {
            MIDIPortDisconnectSource(controllerInput, previous)
            // Release sustain and notes from the departing controller before detach.
            for channel in UInt8(0)..<16 { sendPacket([0xB0 | channel, 64, 0]); sendPacket([0xB0 | channel, 123, 0]) }
        }
        controllerSource = 0; controllerName = "None"; controllerParser = MIDIStream()
    }
    func disconnect() {
        stopController()
        generation &+= 1
        if source != 0 { MIDIPortDisconnectSource(input, source) }
        source = 0; destination = 0; scheduler.clear(); label = "Not connected"
        receiveQueue.sync { parser = MIDIStream(); receiveGeneration = generation }
        onChange?()
    }
    func refresh() {
        #if DEBUG
        NSLog("SM FM1 MIDI: available=\(available) error=\(initializationError ?? "none") inputs=\(ports(input:true).map { $0.name }) outputs=\(ports(input:false).map { $0.name })")
        for port in ports(input:true) {
            var entity=MIDIEntityRef(), device=MIDIDeviceRef(), vp:Int32=0, offline:Int32=0
            MIDIEndpointGetEntity(port.endpoint,&entity); MIDIEntityGetDevice(entity,&device)
            MIDIObjectGetIntegerProperty(device,"USBVendorProduct" as CFString,&vp)
            MIDIObjectGetIntegerProperty(device,kMIDIPropertyOffline,&offline)
            NSLog("SM FM1 endpoint=\(port.name) transport=\(port.transport) device=\(property(device,kMIDIPropertyName)) model=\(property(device,kMIDIPropertyModel)) maker=\(property(device,kMIDIPropertyManufacturer)) vendorProduct=\(vp) offline=\(offline)")
        }

        #endif
        if controllerSource != 0 && !ports(input: true).contains(where: {$0.endpoint == controllerSource}) { stopController() }
        if source != 0 && (!ports(input: true).contains(where: {$0.endpoint == source}) || !ports(input: false).contains(where: {$0.endpoint == destination})) { disconnect() }
        onChange?()
    }
    deinit {
        timer?.invalidate()
        if source != 0 { MIDIPortDisconnectSource(input, source) }
        if controllerInput != 0 { MIDIPortDispose(controllerInput) }
        if input != 0 { MIDIPortDispose(input) }; if output != 0 { MIDIPortDispose(output) }
        if client != 0 { MIDIClientDispose(client) }; packetStorage.deallocate()
    }
}
