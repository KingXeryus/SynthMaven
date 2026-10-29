// SPDX-License-Identifier: GPL-3.0-only
import UIKit
import WebKit
import CoreAudioKit

@main final class AppDelegate: UIResponder, UIApplicationDelegate {
    var window: UIWindow?
    func application(_ application: UIApplication, didFinishLaunchingWithOptions options: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        let window = UIWindow(frame: UIScreen.main.bounds)
        window.rootViewController = UINavigationController(rootViewController: EditorController())
        window.makeKeyAndVisible(); self.window = window; return true
    }
    func applicationDidEnterBackground(_ application: UIApplication) {
        (window?.rootViewController as? UINavigationController)?.viewControllers.compactMap{$0 as? EditorController}.first?.suspend()
    }
}

final class EditorController: UIViewController, WKScriptMessageHandlerWithReply, WKNavigationDelegate {
    let midi = MIDITransport()
    var web: WKWebView!
    var bundleRoot: URL!
    private var exportFile: URL?
    override func viewDidLoad() {
        super.viewDidLoad()
        title = "SynthMaven"
        view.backgroundColor = .systemBackground
        navigationItem.rightBarButtonItems = [UIBarButtonItem(title:"MIDI ports",style:.plain,target:self,action:#selector(connections)), UIBarButtonItem(title:"Bluetooth",style:.plain,target:self,action:#selector(bluetooth))]
        navigationItem.leftBarButtonItem = UIBarButtonItem(title:"Offline",style:.plain,target:self,action:#selector(disconnect))
        let config = WKWebViewConfiguration()
        config.userContentController.addScriptMessageHandler(self, contentWorld: .page, name: "midi")
        let script = try! String(contentsOf: Bundle.main.url(forResource:"MIDIBridge",withExtension:"js")!,encoding:.utf8)
        config.userContentController.addUserScript(WKUserScript(source:script,injectionTime:.atDocumentStart,forMainFrameOnly:true))
        web = WKWebView(frame:.zero,configuration:config);web.navigationDelegate=self;web.translatesAutoresizingMaskIntoConstraints=false
        if #available(iOS 16.4, *) { web.isInspectable = true }
        view.addSubview(web)
        NSLayoutConstraint.activate([web.leadingAnchor.constraint(equalTo:view.leadingAnchor),web.trailingAnchor.constraint(equalTo:view.trailingAnchor),web.topAnchor.constraint(equalTo:view.safeAreaLayoutGuide.topAnchor),web.bottomAnchor.constraint(equalTo:view.safeAreaLayoutGuide.bottomAnchor)])
        bundleRoot = Bundle.main.url(forResource:"Editor",withExtension:nil)!
        let index = bundleRoot.appendingPathComponent("webapp/editor/index.html")
        web.loadFileURL(index,allowingReadAccessTo:bundleRoot)
        midi.onMessage = { [weak self] data in self?.js("receive", data) }
        midi.onChange = { [weak self] in self?.state() }
        midi.onError = { [weak self] text in self?.js("error",text);self?.alert(text) }
    }
    func suspend() { midi.disconnect() }
    @objc func disconnect() { midi.disconnect() }
    private func js(_ method:String,_ value:Any) {
        guard let data=try? JSONSerialization.data(withJSONObject:[value]),let json=String(data:data,encoding:.utf8) else{return}
        web.evaluateJavaScript("window.SynthMavenNative?.\(method)(...\(json))",completionHandler:nil)
    }
    private func state() {
        js("state",["connected":midi.source != 0,"session":midi.sessionID])
        navigationItem.leftBarButtonItem?.title = midi.source != 0 ? "Disconnect" : "Offline"
        navigationItem.leftBarButtonItem?.accessibilityLabel = midi.label
    }
    func alert(_ message:String) {
        guard presentedViewController == nil else{return}
        let a=UIAlertController(title:"SynthMaven",message:message,preferredStyle:.alert)
        a.addAction(UIAlertAction(title:"OK",style:.default));present(a,animated:true)
    }
    @objc func connections() { chooseInput(wireless:false) }
    private func chooseInput(wireless:Bool) {
        let ports=midi.ports(input:true)
        let a=UIAlertController(title:"MIDI input",message:ports.isEmpty ? "Connect the FM-1 by USB in COMPUTER mode. A powered adapter or hub may be needed. For a wireless bridge, pair it with Bluetooth first." : "Choose the FM-1 or its MIDI bridge. Current connection: \(midi.label)",preferredStyle:.actionSheet)
        for port in ports {a.addAction(UIAlertAction(title:port.name,style:.default){[weak self] _ in self?.chooseOutput(port,wireless:wireless || port.transport.lowercased().contains("bluetooth"))})}
        a.addAction(UIAlertAction(title:"Cancel",style:.cancel));a.popoverPresentationController?.barButtonItem=navigationItem.rightBarButtonItems?.first;present(a,animated:true)
    }
    private func chooseOutput(_ input:MIDIPortChoice,wireless:Bool) {
        let a=UIAlertController(title:"MIDI output",message:"Choose the output belonging to \(input.name). The editor verifies the FM-1 identity before loading controls.",preferredStyle:.actionSheet)
        for port in midi.ports(input:false) { a.addAction(UIAlertAction(title:port.name,style:.default){[weak self] _ in
            guard let self else{return};do {try self.midi.connect(source:input,destination:port,wireless:wireless);self.alert("MIDI ports selected. Tap Connect in the editor to read the FM-1.\n\(self.midi.label)")}catch{self.alert(error.localizedDescription)}
        }) }
        a.addAction(UIAlertAction(title:"Cancel",style:.cancel));a.popoverPresentationController?.barButtonItem=navigationItem.rightBarButtonItems?.first;present(a,animated:true)
    }
    @objc func bluetooth() {
        let a=UIAlertController(title:"Bluetooth MIDI",message:"FM-1 firmware 2.2.4 has Bluetooth disabled. Wireless editing needs a bidirectional SysEx-capable Bluetooth MIDI bridge connected to the FM-1’s USB port, or future validated BLE editor firmware. Pairing a pedal alone does not provide an editor connection.",preferredStyle:.alert)
        a.addAction(UIAlertAction(title:"Pair MIDI device",style:.default){[weak self] _ in self?.showPairing()})
        a.addAction(UIAlertAction(title:"Use paired bridge",style:.default){[weak self] _ in self?.chooseInput(wireless:true)})
        a.addAction(UIAlertAction(title:"Cancel",style:.cancel));present(a,animated:true)
    }
    private func showPairing() {
        let controller=CABTMIDICentralViewController()
        controller.navigationItem.rightBarButtonItem=UIBarButtonItem(title:"Done",style:.done,target:self,action:#selector(closePairing))
        present(UINavigationController(rootViewController:controller),animated:true)
    }
    @objc func closePairing() { dismiss(animated:true){self.midi.refresh();self.chooseInput(wireless:true)} }
    func userContentController(_ userContentController:WKUserContentController,didReceive message:WKScriptMessage,replyHandler:@escaping(Any?,String?)->Void) {
        guard message.frameInfo.isMainFrame,let url=message.frameInfo.request.url,url.isFileURL,
              url.standardizedFileURL.path == bundleRoot.appendingPathComponent("webapp/editor/index.html").standardizedFileURL.path,
              let body=message.body as? [String:Any],let action=body["action"] as? String else{replyHandler(nil,"Untrusted editor page");return}
        do {
            switch action {
            case "status":replyHandler(["connected":midi.source != 0,"session":midi.sessionID],nil);return
            case "disconnect":midi.disconnect()
            case "send":
                guard let session=body["session"] as? UInt,session==midi.sessionID,let values=body["bytes"] as? [Int],values.allSatisfy({0...255 ~= $0}) else{throw MIDIProblem.invalid("Invalid bytes")}
                try midi.send(values.map(UInt8.init))
            case "export":
                guard let name=body["name"] as? String,let encoded=body["data"] as? String,encoded.count<24*1024*1024,let data=Data(base64Encoded:encoded),data.count<=16*1024*1024 else{throw MIDIProblem.invalid("Invalid export")}
                let folder=FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
                try FileManager.default.createDirectory(at:folder,withIntermediateDirectories:true)
                let file=folder.appendingPathComponent(URL(fileURLWithPath:name).lastPathComponent)
                try data.write(to:file,options:.atomic)
                let share=UIActivityViewController(activityItems:[file],applicationActivities:nil)
                share.popoverPresentationController?.sourceView=view;share.popoverPresentationController?.sourceRect=CGRect(x:view.bounds.midX,y:0,width:1,height:1)
                share.completionWithItemsHandler={_,_,_,_ in try? FileManager.default.removeItem(at:folder)}
                present(share,animated:true)
            case "error":alert(body["message"] as? String ?? "Export failed")
            default:throw MIDIProblem.invalid("Unknown bridge action")
            }
            replyHandler(true,nil)
        }catch{replyHandler(nil,error.localizedDescription)}
    }
    func webView(_ webView:WKWebView,didFinish navigation:WKNavigation!){state()}
    func webView(_ webView:WKWebView,decidePolicyFor navigationAction:WKNavigationAction,decisionHandler:@escaping(WKNavigationActionPolicy)->Void) {
        guard let url=navigationAction.request.url else{decisionHandler(.cancel);return}
        if url.isFileURL && url.standardizedFileURL.path.hasPrefix(bundleRoot.path+"/") {decisionHandler(.allow);return}
        if navigationAction.navigationType == .linkActivated && ["https","http"].contains(url.scheme ?? "") {UIApplication.shared.open(url)}
        decisionHandler(.cancel)
    }
}
