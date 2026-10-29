import Foundation
func check(_ condition: @autoclosure () -> Bool,_ text:String){if !condition(){fatalError(text)};print("PASS: \(text)")}
var parser=MIDIStream()
check(parser.feed([0xF0,125,70]) == [],"partial SysEx waits")
check(parser.feed([0xF8,76,1,0xF7]) == [[0xF8],[0xF0,125,70,76,1,0xF7]],"fragmented SysEx survives realtime bytes")
check(parser.feed([0x90,60,100,61,110,0x80,60,0]) == [[0x90,60,100],[0x90,61,110],[0x80,60,0]],"running status and note-off")
check(parser.feed([0xC0,5,6]) == [[0xC0,5],[0xC0,6]],"two-byte channel messages")
var queue=MIDIQueue();let info:[UInt8]=[0xF0,125,70,76,1,0xF7]
try queue.append(info);try queue.append(info)
check(queue.take(now:0)==info,"first complete frame")
check(queue.take(now:0.039)==nil,"USB rate bound")
check(queue.take(now:0.04)==info,"USB frame ordering")
queue.interval=0.08;queue.clear();try queue.append(info);try queue.append(info)
check(queue.take(now:1)==info && queue.take(now:1.079)==nil && queue.take(now:1.08)==info,"Bluetooth rate bound")
queue.clear();for _ in 0..<32 {try queue.append(info)}
do{try queue.append(info);fatalError("overflow accepted")}catch{print("PASS: queue overflow rejected")}
queue.clear();check(queue.take(now:99)==nil,"disconnect discards stale writes")
for bad:[UInt8] in [[0xF0,1,2,0xF7],[0xF0,125,70,76,1],[0x90,60,255],[0xF8]] {
 do{try queue.append(bad);fatalError("invalid message accepted")}catch{}
}
print("PASS: malformed messages and non-editor SysEx rejected")
