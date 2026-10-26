#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Single-scene UIKit lifecycle bridge for pinned JUCE 7 on iOS SDK 27+.
Keeps JUCE startup and window ownership, but starts UI after a UIWindowScene exists.
"""
from pathlib import Path
import sys
root=Path(sys.argv[1])/'modules/juce_gui_basics/native'
p=root/'juce_UIViewComponentPeer_ios.mm';s=p.read_text()
if 'synthMavenWindowScene' not in s:
 s=s.replace('namespace juce\n{','static UIWindowScene* synthMavenWindowScene = nil; // SynthMaven single-scene bridge\n\nnamespace juce\n{',1)
 s=s.replace('window = [[JuceUIWindow alloc] initWithFrame: r];','window = [[JuceUIWindow alloc] initWithFrame: r];\n        if (synthMavenWindowScene != nil) window.windowScene = synthMavenWindowScene;',1)
 p.write_text(s)
p=root/'juce_Windowing_ios.mm';s=p.read_text()
if '@interface SynthMavenSceneDelegate' not in s:
 s=s.replace('- (void) applicationDidFinishLaunching: (UIApplication*) application;','- (void) applicationDidFinishLaunching: (UIApplication*) application;\n- (void) synthMavenStart: (UIApplication*) application;',1)
 s=s.replace('@implementation JuceAppStartupDelegate','''@interface SynthMavenSceneDelegate : UIResponder <UIWindowSceneDelegate>
@end
@implementation SynthMavenSceneDelegate
- (void) scene: (UIScene*) scene willConnectToSession: (UISceneSession*) session options: (UISceneConnectionOptions*) options
{
    (void) session; (void) options;
    if (![scene isKindOfClass: [UIWindowScene class]]) return;
    synthMavenWindowScene = (UIWindowScene*) scene;
    JuceAppStartupDelegate* app = (JuceAppStartupDelegate*) [UIApplication sharedApplication].delegate;
    if (juce::JUCEApplicationBase::getInstance() == nullptr)
        [app synthMavenStart: [UIApplication sharedApplication]];
    else
        for (int i = 0; i < juce::Desktop::getInstance().getNumComponents(); ++i)
            if (auto* peer = juce::Desktop::getInstance().getComponent(i)->getPeer())
            {
                UIView* view = (UIView*) peer->getNativeHandle();
                if (view.window != nil) view.window.windowScene = synthMavenWindowScene;
            }
}
- (void) sceneDidBecomeActive: (UIScene*) scene
{ (void) scene; [(JuceAppStartupDelegate*) [UIApplication sharedApplication].delegate applicationDidBecomeActive: [UIApplication sharedApplication]]; }
- (void) sceneWillResignActive: (UIScene*) scene
{ (void) scene; [(JuceAppStartupDelegate*) [UIApplication sharedApplication].delegate applicationWillResignActive: [UIApplication sharedApplication]]; }
- (void) sceneWillEnterForeground: (UIScene*) scene
{ (void) scene; [(JuceAppStartupDelegate*) [UIApplication sharedApplication].delegate applicationWillEnterForeground: [UIApplication sharedApplication]]; }
- (void) sceneDidEnterBackground: (UIScene*) scene
{ (void) scene; [(JuceAppStartupDelegate*) [UIApplication sharedApplication].delegate applicationDidEnterBackground: [UIApplication sharedApplication]]; }
- (void) sceneDidDisconnect: (UIScene*) scene
{ if (scene == synthMavenWindowScene) synthMavenWindowScene = nil; }
@end

@implementation JuceAppStartupDelegate''',1)
 s=s.replace('- (void) applicationDidFinishLaunching: (UIApplication*) application\n{','''- (void) applicationDidFinishLaunching: (UIApplication*) application
{
    (void) application; // UI startup is deferred to UIWindowScene connection.
}

- (void) synthMavenStart: (UIApplication*) application
{''',1)
 s=s.replace('UIWindow* window = [[UIWindow alloc] init];','UIWindow* window = synthMavenWindowScene != nil ? [[UIWindow alloc] initWithWindowScene: synthMavenWindowScene] : [[UIWindow alloc] init];',1)
 p.write_text(s)
if 'configurationForConnectingSceneSession' not in s:
 s=s.replace('@implementation JuceAppStartupDelegate',"""@implementation JuceAppStartupDelegate
- (UISceneConfiguration*) application: (UIApplication*) application configurationForConnectingSceneSession: (UISceneSession*) session options: (UISceneConnectionOptions*) options
{
    (void) application; (void) options;
    UISceneConfiguration* config = [[[UISceneConfiguration alloc] initWithName: @"SynthMaven" sessionRole: session.role] autorelease];
    config.sceneClass = [UIWindowScene class];
    config.delegateClass = [SynthMavenSceneDelegate class];
    return config;
}
""",1)
 p.write_text(s)
print('JUCE iOS scene lifecycle bridge ready')
