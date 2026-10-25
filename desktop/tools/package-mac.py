#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Create a universal Mac installer PKG and DMG.
Optional: SYNTHMAVEN_SIGN_ID (Developer ID Application), SYNTHMAVEN_INSTALLER_ID
(Developer ID Installer), SYNTHMAVEN_NOTARY_PROFILE (existing keychain profile).
No passwords or private keys are read by this script.
"""
import pathlib,subprocess,os,shutil,sys,plistlib,tempfile
from xml.etree import ElementTree as ET
ROOT=pathlib.Path(__file__).resolve().parents[2]
OUT=pathlib.Path(sys.argv[1]).resolve() if len(sys.argv)>1 else ROOT/'build/mac-distribution'
OUT.mkdir(parents=True,exist_ok=True)
WORK=pathlib.Path(tempfile.mkdtemp(prefix='SynthMaven-package-',dir='/private/tmp'))
STAGE=WORK/'root';DMGROOT=WORK/'dmg'
for p in (STAGE,DMGROOT):
 if p.exists():shutil.rmtree(p)
 p.mkdir(parents=True)
def run(*args):subprocess.run(list(map(str,args)),check=True)
for fmt,sub,name in [('Standalone','Applications','SynthMaven.app'),('AU','Library/Audio/Plug-Ins/Components','SynthMaven.component'),('VST3','Library/Audio/Plug-Ins/VST3','SynthMaven.vst3')]:
 dest=STAGE/sub/name;dest.parent.mkdir(parents=True,exist_ok=True)
 run('ditto','--norsrc',ROOT/'build/desktop/SynthMaven_artefacts/Release'/fmt/name,dest)
 run('xattr','-cr',dest)
 identity=os.environ.get('SYNTHMAVEN_SIGN_ID','-')
 args=['codesign','--force','--sign',identity]
 if identity!='-':args+=['--timestamp','--options','runtime']
 run(*args,dest);run('codesign','--verify','--strict',dest)
version='0.2.0';signed=bool(os.environ.get('SYNTHMAVEN_SIGN_ID'));distribution_signed=signed and bool(os.environ.get('SYNTHMAVEN_INSTALLER_ID'));suffix='' if distribution_signed else '-Local-Test'
pkg=DMGROOT/f'SynthMaven-{version}{suffix}.pkg'
# Separate component packages let the user choose the DAW formats explicitly.
component_dir=WORK/'packages';component_dir.mkdir()
formats=[('standalone','Standalone application','Applications','SynthMaven.app'),
 ('vst3','VST3 instrument','Library/Audio/Plug-Ins/VST3','SynthMaven.vst3'),
 ('au','Audio Unit instrument (Logic Pro / GarageBand)','Library/Audio/Plug-Ins/Components','SynthMaven.component')]
# VST2 is included only when a separately licensed build exists, never renamed VST3.
legacy=ROOT/'build/desktop/SynthMaven_artefacts/Release/VST/SynthMaven.vst'
if legacy.exists():
 dest=STAGE/'Library/Audio/Plug-Ins/VST/SynthMaven.vst';dest.parent.mkdir(parents=True,exist_ok=True)
 run('ditto','--norsrc',legacy,dest);run('xattr','-cr',dest)
 args=['codesign','--force','--sign',os.environ.get('SYNTHMAVEN_SIGN_ID','-')]
 if signed:args+=['--timestamp','--options','runtime']
 run(*args,dest);run('codesign','--verify','--strict',dest)
 formats.append(('vst2','Legacy VST2 instrument','Library/Audio/Plug-Ins/VST','SynthMaven.vst'))
distribution=ET.Element('installer-gui-script',{'minSpecVersion':'2'})
ET.SubElement(distribution,'title').text='SynthMaven '+version
ET.SubElement(distribution,'options',{'customize':'always','require-scripts':'false','hostArchitectures':'arm64,x86_64'})
ET.SubElement(distribution,'domains',{'enable_anywhere':'false','enable_currentUserHome':'false','enable_localSystem':'true'})
outline=ET.SubElement(distribution,'choices-outline')
for kind,title,sub,name in formats:
 component_root=WORK/('root-'+kind);dest=component_root/sub/name;dest.parent.mkdir(parents=True,exist_ok=True)
 run('ditto','--norsrc',STAGE/sub/name,dest)
 config=WORK/(kind+'.plist');run('pkgbuild','--analyze','--root',component_root,config)
 items=plistlib.loads(config.read_bytes())
 for item in items:item['BundleIsRelocatable']=False
 config.write_bytes(plistlib.dumps(items))
 identifier='com.synthmaven.instrument.'+kind;filename='SynthMaven-'+kind+'.pkg'
 args=['pkgbuild','--component-plist',config,'--root',component_root,'--identifier',identifier,'--version',version,'--install-location','/',component_dir/filename]
 run(*args)
 ET.SubElement(outline,'line',{'choice':kind})
 choice=ET.SubElement(distribution,'choice',{'id':kind,'title':title,'start_selected':'true','visible':'true','enabled':'true'})
 ET.SubElement(choice,'pkg-ref',{'id':identifier})
 reference=ET.SubElement(distribution,'pkg-ref',{'id':identifier,'version':version,'onConclusion':'none'});reference.text=filename
xml=WORK/'Distribution.xml';ET.ElementTree(distribution).write(xml,encoding='utf-8',xml_declaration=True)
args=['productbuild','--distribution',xml,'--package-path',component_dir]
if os.environ.get('SYNTHMAVEN_INSTALLER_ID'):args+=['--sign',os.environ['SYNTHMAVEN_INSTALLER_ID']]
run(*args,pkg)
shutil.copy2(ROOT/'desktop/INSTALL.txt',DMGROOT/'Read Me.txt')
shutil.copy2(ROOT/'LICENSE',DMGROOT/'GPL-3.0.txt')
shutil.copy2(ROOT/'desktop/THIRD_PARTY.txt',DMGROOT/'Third Party Notices.txt')
source=OUT/'SynthMaven-Software-0.2.0-Source.zip'
run(sys.executable,ROOT/'desktop/tools/source-archive.py',source)
shutil.copy2(source,DMGROOT/source.name)
dmg=OUT/f'SynthMaven-{version}-macOS-Universal{suffix}.dmg'
run('hdiutil','create','-volname','SynthMaven 0.2.0','-srcfolder',DMGROOT,'-format','UDZO','-ov',dmg)
if signed:run('codesign','--sign',os.environ['SYNTHMAVEN_SIGN_ID'],'--timestamp',dmg)
profile=os.environ.get('SYNTHMAVEN_NOTARY_PROFILE')
if profile:
 if not signed or not os.environ.get('SYNTHMAVEN_INSTALLER_ID'):raise SystemExit('Notarization requires both Developer ID signing identities')
 run('xcrun','notarytool','submit',dmg,'--keychain-profile',profile,'--wait')
 run('xcrun','stapler','staple',dmg);run('spctl','--assess','--type','open','--context','context:primary-signature','-v',dmg)
shutil.copy2(pkg,OUT/pkg.name)
print(dmg)
shutil.rmtree(WORK)
