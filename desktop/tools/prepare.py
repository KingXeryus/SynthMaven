#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Fetch the pinned GPL JUCE 7 sources, apply the macOS SDK compatibility patch,
and generate DSP headers/context. Requires Python 3, Pillow, Xcode CLT and curl.
"""
import hashlib, pathlib, subprocess, sys, tarfile, os
ROOT=pathlib.Path(__file__).resolve().parents[2]
JUCE=ROOT/'desktop/vendor/JUCE-7.0.12'
SHA='94e3b35e1990cd67f59736bb5e1eff1cfd9590b16fea82696f309a8f59452434'
os.environ.setdefault('DEVELOPER_DIR','/Library/Developer/CommandLineTools')
if not JUCE.exists():
 archive=ROOT/'build/JUCE-7.0.12.tar.gz';archive.parent.mkdir(parents=True,exist_ok=True)
 subprocess.run(['curl','-fL','https://codeload.github.com/juce-framework/JUCE/tar.gz/refs/tags/7.0.12','-o',str(archive)],check=True)
 if hashlib.sha256(archive.read_bytes()).hexdigest()!=SHA:raise SystemExit('JUCE archive checksum mismatch')
 JUCE.parent.mkdir(parents=True,exist_ok=True)
 with tarfile.open(archive) as tf:tf.extractall(JUCE.parent,filter='data')
p=JUCE/'modules/juce_gui_basics/native/juce_Windowing_mac.mm';s=p.read_text()
if 'CGWindowListCreateImage (CGRectNull' in s:
 a=s.index('static Image createNSWindowSnapshot');b=s.index('\nImage createSnapshotOfNativeWindow',a)
 s=s[:a]+'''// SynthMaven SDK compatibility: snapshot only our own NSView.
static Image createNSWindowSnapshot (NSWindow* nsWindow)
{
    NSView* view = [nsWindow contentView];
    if (view == nil) return {};
    NSBitmapImageRep* rep = [view bitmapImageRepForCachingDisplayInRect: [view bounds]];
    if (rep == nil) return {};
    [view cacheDisplayInRect: [view bounds] toBitmapImageRep: rep];
    NSData* png = [rep representationUsingType: NSBitmapImageFileTypePNG properties: @{}];
    return ImageFileFormat::loadFrom ([png bytes], [png length]);
}
'''+s[b:];p.write_text(s)
sys.path.insert(0,str(ROOT/'tools'));import build
build.generate()
subprocess.run([sys.executable,str(ROOT/'desktop/tools/contextualize.py'),str(ROOT/'build/desktop-generated')],check=True)
