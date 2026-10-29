#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/ios-tests
python3 ios/prepare_editor.py
swiftc ios/SynthMavenEditor/MIDIStream.swift ios/Tests/main.swift -o build/ios-tests/midi
build/ios-tests/midi
node ios/Tests/bridge.test.mjs
node ios/Tests/perform.test.mjs
xcodebuild -project ios/SynthMavenEditor.xcodeproj -scheme SynthMavenEditor -destination 'generic/platform=iOS Simulator' -derivedDataPath build/ios CODE_SIGNING_ALLOWED=NO build
