// SPDX-License-Identifier: GPL-3.0-only
import UIKit
import WebKit
import CoreAudioKit

@main final class AppDelegate: UIResponder, UIApplicationDelegate {
    func application(_ application: UIApplication, configurationForConnecting session: UISceneSession, options: UIScene.ConnectionOptions) -> UISceneConfiguration {
        let configuration = UISceneConfiguration(name: "Editor", sessionRole: session.role)
        configuration.delegateClass = EditorSceneDelegate.self
        return configuration
    }
}

final class EditorSceneDelegate: UIResponder, UIWindowSceneDelegate {
    var window: UIWindow?
    func scene(_ scene: UIScene, willConnectTo session: UISceneSession, options: UIScene.ConnectionOptions) {
        guard let scene = scene as? UIWindowScene else { return }
        let window = UIWindow(windowScene: scene)
        window.overrideUserInterfaceStyle = .dark
        window.rootViewController = UINavigationController(rootViewController: EditorController())
        window.makeKeyAndVisible(); self.window = window
    }
    func sceneDidEnterBackground(_ scene: UIScene) {
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
        title = "SynthMaven FM1"
        view.backgroundColor = .systemBackground
        navigationItem.rightBarButtonItem = UIBarButtonItem(title:"Connect",style:.plain,target:self,action:#selector(connections))
        navigationItem.leftBarButtonItem = nil
        let config = WKWebViewConfiguration()
        config.userContentController.addScriptMessageHandler(self, contentWorld: .page, name: "midi")
        let script = try! String(contentsOf: Bundle.main.url(forResource:"MIDIBridge",withExtension:"js")!,encoding:.utf8)
        config.userContentController.addUserScript(WKUserScript(source:script,injectionTime:.atDocumentStart,forMainFrameOnly:true))
        web = WKWebView(frame:.zero,configuration:config);web.navigationDelegate=self;web.translatesAutoresizingMaskIntoConstraints=false
        #if DEBUG
        if #available(iOS 16.4, *) { web.isInspectable = true }
        #endif
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
        js("state",["connected":midi.source != 0,"session":midi.sessionID,"controller":midi.controllerName])
        navigationItem.rightBarButtonItem?.title = midi.source != 0 ? "Connection" : "Connect"
        navigationItem.leftBarButtonItem?.accessibilityLabel = midi.label
    }
    func alert(_ message:String) {
        guard presentedViewController == nil else{return}
        let a=UIAlertController(title:"SynthMaven",message:message,preferredStyle:.alert)
        a.addAction(UIAlertAction(title:"OK",style:.default));present(a,animated:true)
    }
    @objc func connections() {
        let sheet = UIAlertController(title:"Connect to your FM-1",message:"Connect by USB. Matching MIDI ports are selected automatically. Direct Bluetooth is not included in this USB-first build.",preferredStyle:.actionSheet)
        sheet.addAction(UIAlertAction(title:"USB cable",style:.default){[weak self] _ in self?.connectDetected(wireless:false)})
        if midi.source != 0 { sheet.addAction(UIAlertAction(title:"Disconnect",style:.destructive){[weak self] _ in self?.midi.disconnect()}) }
        sheet.addAction(UIAlertAction(title:"Cancel",style:.cancel))
        sheet.popoverPresentationController?.barButtonItem=navigationItem.rightBarButtonItem;present(sheet,animated:true)
    }
    private func chooseController() {
        guard midi.source != 0 else { alert("Connect the FM-1 first."); return }
        let sheet = UIAlertController(title:"Controller for MIDI Learn", message:"Connect the controller and FM-1 to the iPad through a powered USB hub. Channel messages are forwarded to the FM-1 while this app is open. Current: \(midi.controllerName)", preferredStyle:.actionSheet)
        sheet.addAction(UIAlertAction(title:"None — stop routing",style:.default){[weak self] _ in try? self?.midi.routeController(nil)})
        for port in midi.ports(input:true) where port.endpoint != midi.source {
            sheet.addAction(UIAlertAction(title:port.name,style:.default){[weak self] _ in
                do { try self?.midi.routeController(port) } catch { self?.alert(error.localizedDescription) }
            })
        }
        sheet.addAction(UIAlertAction(title:"Cancel",style:.cancel))
        sheet.popoverPresentationController?.barButtonItem=navigationItem.rightBarButtonItem
        present(sheet,animated:true)
    }
    private func connectDetected(wireless:Bool) {
        midi.refresh()
        let inputs=midi.ports(input:true), outputs=midi.ports(input:false)
        let candidates=inputs.filter { port in
            let name=port.name.lowercased()
            return (name.contains("synthmaven") || name.contains("fm-1") || name.contains("melodee") || name.contains("felucca")) && (wireless == (name.contains("bluetooth") || port.transport.lowercased().contains("bluetooth")))
        }
        func open(_ source:MIDIPortChoice) {
            let matches=outputs.filter{$0.name==source.name}
            guard matches.count==1,let output=matches.first else { chooseOutput(source,wireless:wireless);return }
            do { try midi.connect(source:source,destination:output,wireless:wireless);js("connect",true) } catch { alert(error.localizedDescription) }
        }
        if candidates.count==1 { open(candidates[0]);return }
        let sheet=UIAlertController(title:candidates.isEmpty ? "FM-1 not found" : "Choose your FM-1",message:candidates.isEmpty ? (wireless ? "Enable GLO → BLE EDITOR on the FM-1, then pair it in Bluetooth." : "Set USB ROLE to COMPUTER and connect a USB data cable or powered adapter.") : "Matching input and output ports will be selected for you.",preferredStyle:.actionSheet)
        for port in candidates {sheet.addAction(UIAlertAction(title:port.name,style:.default){_ in open(port)})}
        sheet.addAction(UIAlertAction(title:"Try again",style:.default){[weak self] _ in if wireless {self?.showPairing()} else {self?.connectDetected(wireless:false)}})
        sheet.addAction(UIAlertAction(title:"Manual MIDI ports",style:.default){[weak self] _ in self?.chooseInput(wireless:wireless)})
        sheet.addAction(UIAlertAction(title:"Cancel",style:.cancel));sheet.popoverPresentationController?.barButtonItem=navigationItem.rightBarButtonItem;present(sheet,animated:true)
    }
    private func chooseInput(wireless:Bool) {
        let ports=midi.ports(input:true)
        let detected=ports.contains { let n=$0.name.lowercased(); return n.contains("fm-1") || n.contains("synthmaven") || n.contains("felucca") }
        let help=detected ? "Choose the FM-1 input, then its matching output." : "FM-1 is not detected by iPadOS. Set it to COMPUTER mode and use a USB data cable; if direct USB-C does not appear, try a USB adapter or powered hub. Network and other instrument ports are not the FM-1. For Bluetooth, pair SynthMaven first."
        let a=UIAlertController(title:"MIDI input",message:help+"\nCurrent connection: \(midi.label)",preferredStyle:.actionSheet)
        for port in ports {a.addAction(UIAlertAction(title:port.name,style:.default){[weak self] _ in self?.chooseOutput(port,wireless:wireless || port.transport.lowercased().contains("bluetooth"))})}
        a.addAction(UIAlertAction(title:"Cancel",style:.cancel));a.popoverPresentationController?.barButtonItem=navigationItem.rightBarButtonItems?.first;present(a,animated:true)
    }
    private func chooseOutput(_ input:MIDIPortChoice,wireless:Bool) {
        let a=UIAlertController(title:"MIDI output",message:"Choose the output belonging to \(input.name). The editor verifies the FM-1 identity before loading controls.",preferredStyle:.actionSheet)
        for port in midi.ports(input:false) { a.addAction(UIAlertAction(title:port.name,style:.default){[weak self] _ in
            guard let self else{return};do {try self.midi.connect(source:input,destination:port,wireless:wireless);self.js("connect",true)}catch{self.alert(error.localizedDescription)}
        }) }
        a.addAction(UIAlertAction(title:"Cancel",style:.cancel));a.popoverPresentationController?.barButtonItem=navigationItem.rightBarButtonItems?.first;present(a,animated:true)
    }
    @objc func bluetooth() {
        let a=UIAlertController(title:"Bluetooth MIDI",message:"Direct FM-1 Bluetooth editing requires compatible firmware with experimental Bluetooth editing enabled. Keep Bluetooth off when not editing. Older firmware needs a bidirectional SysEx-capable MIDI bridge. Pairing alone does not establish compatibility; Connect verifies the FM-1 protocol.",preferredStyle:.alert)
        a.addAction(UIAlertAction(title:"Pair MIDI device",style:.default){[weak self] _ in self?.showPairing()})
        a.addAction(UIAlertAction(title:"Use paired device",style:.default){[weak self] _ in self?.chooseInput(wireless:true)})
        a.addAction(UIAlertAction(title:"Cancel",style:.cancel));present(a,animated:true)
    }
    private func showPairing() {
        let controller=CABTMIDICentralViewController()
        controller.navigationItem.rightBarButtonItem=UIBarButtonItem(title:"Done",style:.done,target:self,action:#selector(closePairing))
        present(UINavigationController(rootViewController:controller),animated:true)
    }
    @objc func closePairing() { dismiss(animated:true){self.connectDetected(wireless:true)} }
    func userContentController(_ userContentController:WKUserContentController,didReceive message:WKScriptMessage,replyHandler:@escaping(Any?,String?)->Void) {
        guard message.frameInfo.isMainFrame,let url=message.frameInfo.request.url,url.isFileURL,
              url.resolvingSymlinksInPath().standardizedFileURL.path == bundleRoot.appendingPathComponent("webapp/editor/index.html").resolvingSymlinksInPath().standardizedFileURL.path,
              let body=message.body as? [String:Any],let action=body["action"] as? String else{replyHandler(nil,"Untrusted editor page");return}
        do {
            switch action {
            case "connect":connections();replyHandler(true,nil);return
            case "routeController":chooseController();replyHandler(true,nil);return
            case "status":replyHandler(["connected":midi.source != 0,"session":midi.sessionID,"controller":midi.controllerName],nil);return
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
    func webView(_ webView:WKWebView,didFailProvisionalNavigation navigation:WKNavigation!,withError error:Error){alert("The editor page could not load. Close and reopen the app.\n"+error.localizedDescription)}
    func webView(_ webView:WKWebView,decidePolicyFor navigationAction:WKNavigationAction,decisionHandler:@escaping(WKNavigationActionPolicy)->Void) {
        guard let url=navigationAction.request.url else{decisionHandler(.cancel);return}
        if url.isFileURL && url.resolvingSymlinksInPath().standardizedFileURL.path.hasPrefix(bundleRoot.resolvingSymlinksInPath().standardizedFileURL.path+"/") {decisionHandler(.allow);return}
        if navigationAction.navigationType == .linkActivated && ["https","http"].contains(url.scheme ?? "") {UIApplication.shared.open(url)}
        decisionHandler(.cancel)
    }
}
