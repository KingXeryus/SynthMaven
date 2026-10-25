#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Create a universal Mac installer PKG and DMG.
Optional: SYNTHMAVEN_SIGN_ID (Developer ID Application), SYNTHMAVEN_INSTALLER_ID
(Developer ID Installer), SYNTHMAVEN_NOTARY_PROFILE (existing keychain profile).
No passwords or private keys are read by this script.
"""
import pathlib,subprocess,os,shutil,sys,plistlib,tempfile
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
version='0.1.0';signed=bool(os.environ.get('SYNTHMAVEN_SIGN_ID'));suffix='' if signed else '-Local-Test'
pkg=DMGROOT/f'SynthMaven-{version}{suffix}.pkg'
components=WORK/'components.plist'
run('pkgbuild','--analyze','--root',STAGE,components)
items=plistlib.loads(components.read_bytes())
for item in items:item['BundleIsRelocatable']=False
components.write_bytes(plistlib.dumps(items))
args=['pkgbuild','--component-plist',components,'--root',STAGE,'--identifier','com.synthmaven.instrument','--version',version,'--install-location','/']
if os.environ.get('SYNTHMAVEN_INSTALLER_ID'):args+=['--sign',os.environ['SYNTHMAVEN_INSTALLER_ID']]
run(*args,pkg)
shutil.copy2(ROOT/'desktop/INSTALL.txt',DMGROOT/'Read Me.txt')
shutil.copy2(ROOT/'LICENSE',DMGROOT/'GPL-3.0.txt')
shutil.copy2(ROOT/'desktop/THIRD_PARTY.txt',DMGROOT/'Third Party Notices.txt')
source=OUT/'SynthMaven-Software-0.1.0-Source.zip'
run(sys.executable,ROOT/'desktop/tools/source-archive.py',source)
shutil.copy2(source,DMGROOT/source.name)
dmg=OUT/f'SynthMaven-{version}-macOS-Universal{suffix}.dmg'
run('hdiutil','create','-volname','SynthMaven 0.1.0','-srcfolder',DMGROOT,'-format','UDZO','-ov',dmg)
if signed:run('codesign','--sign',os.environ['SYNTHMAVEN_SIGN_ID'],'--timestamp',dmg)
profile=os.environ.get('SYNTHMAVEN_NOTARY_PROFILE')
if profile:
 if not signed or not os.environ.get('SYNTHMAVEN_INSTALLER_ID'):raise SystemExit('Notarization requires both Developer ID signing identities')
 run('xcrun','notarytool','submit',dmg,'--keychain-profile',profile,'--wait')
 run('xcrun','stapler','staple',dmg);run('spctl','--assess','--type','open','--context','context:primary-signature','-v',dmg)
shutil.copy2(pkg,OUT/pkg.name)
print(dmg)
shutil.rmtree(WORK)
