#!/usr/bin/env python3
from pathlib import Path
import hashlib,plistlib
root=Path(__file__).parent
uid=lambda s:hashlib.sha1(s.encode()).hexdigest()[:24].upper()
objects={}
def add(name,value):objects[uid(name)]=value;return uid(name)
files=['App.swift','MIDITransport.swift','MIDIStream.swift','MIDIBridge.js']
refs=[];sources=[];resources=[]
for f in files:
 ref=add(f,f'isa = PBXFileReference; lastKnownFileType = {"sourcecode.swift" if f.endswith("swift") else "sourcecode.javascript"}; path = SynthMavenEditor/{f}; sourceTree = "<group>";')
 build=add(f+'build',f'isa = PBXBuildFile; fileRef = {ref};');refs.append(ref);(sources if f.endswith('swift') else resources).append(build)
ref=add('Editor','isa = PBXFileReference; lastKnownFileType = folder; path = Generated/Editor; sourceTree = "<group>";');refs.append(ref);resources.append(add('Editorbuild',f'isa = PBXBuildFile; fileRef = {ref};'))
ref=add('Assets','isa = PBXFileReference; lastKnownFileType = folder.assetcatalog; path = SynthMavenEditor/Assets.xcassets; sourceTree = "<group>";');refs.append(ref);resources.append(add('Assetsbuild',f'isa = PBXBuildFile; fileRef = {ref};'))
product=add('Product','isa = PBXFileReference; explicitFileType = wrapper.application; path = SynthMavenEditor.app; sourceTree = BUILT_PRODUCTS_DIR;')
products=add('Products',f'isa = PBXGroup; children = ({product},); name = Products; sourceTree = "<group>";')
group=add('Group',f'isa = PBXGroup; children = ({",".join(refs+[products])},); sourceTree = "<group>";')
sourcephase=add('Sources',f'isa = PBXSourcesBuildPhase; buildActionMask = 2147483647; files = ({",".join(sources)},); runOnlyForDeploymentPostprocessing = 0;')
resourcephase=add('Resources',f'isa = PBXResourcesBuildPhase; buildActionMask = 2147483647; files = ({",".join(resources)},); runOnlyForDeploymentPostprocessing = 0;')
frameworkphase=add('Frameworks','isa = PBXFrameworksBuildPhase; buildActionMask = 2147483647; files = (); runOnlyForDeploymentPostprocessing = 0;')
configs=[];pconfigs=[]
for config in ['Debug','Release']:
 settings='SWIFT_VERSION = 5.0; IPHONEOS_DEPLOYMENT_TARGET = 16.0; TARGETED_DEVICE_FAMILY = "2"; SDKROOT = iphoneos; SUPPORTED_PLATFORMS = "iphoneos iphonesimulator"; PRODUCT_BUNDLE_IDENTIFIER = com.synthmaven.fm1editor; PRODUCT_NAME = "$(TARGET_NAME)"; INFOPLIST_FILE = SynthMavenEditor/Info.plist; CODE_SIGN_STYLE = Automatic; CURRENT_PROJECT_VERSION = 15; ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon; MARKETING_VERSION = 3.5.2; ENABLE_USER_SCRIPT_SANDBOXING = YES; SWIFT_OPTIMIZATION_LEVEL = '+ ('"-Onone";' if config=='Debug' else '"-O";')
 if config=='Debug': settings+=' SWIFT_ACTIVE_COMPILATION_CONDITIONS = DEBUG;'
 configs.append(add('target'+config,f'isa = XCBuildConfiguration; name = {config}; buildSettings = {{{settings}}};'))
 pconfigs.append(add('project'+config,f'isa = XCBuildConfiguration; name = {config}; buildSettings = {{ CLANG_ENABLE_MODULES = YES; }};'))
tcfg=add('TargetConfig',f'isa = XCConfigurationList; buildConfigurations = ({",".join(configs)},); defaultConfigurationIsVisible = 0; defaultConfigurationName = Release;')
pcfg=add('ProjectConfig',f'isa = XCConfigurationList; buildConfigurations = ({",".join(pconfigs)},); defaultConfigurationIsVisible = 0; defaultConfigurationName = Release;')
target=add('Target',f'isa = PBXNativeTarget; buildConfigurationList = {tcfg}; buildPhases = ({sourcephase},{frameworkphase},{resourcephase},); buildRules = (); dependencies = (); name = SynthMavenEditor; productName = SynthMavenEditor; productReference = {product}; productType = "com.apple.product-type.application";')
project=add('Project',f'isa = PBXProject; attributes = {{ LastUpgradeCheck = 1600; }}; buildConfigurationList = {pcfg}; compatibilityVersion = "Xcode 14.0"; developmentRegion = en; hasScannedForEncodings = 0; knownRegions = (en, Base,); mainGroup = {group}; productRefGroup = {products}; projectDirPath = ""; projectRoot = ""; targets = ({target},);')
p=root/'SynthMavenEditor.xcodeproj';p.mkdir(exist_ok=True)
(p/'project.pbxproj').write_text('// !$*UTF8*$!\n{ archiveVersion = 1; classes = {}; objectVersion = 56; objects = {\n'+''.join(k+' = {'+v+'};\n' for k,v in objects.items())+'}; rootObject = '+project+'; }\n')
scheme=p/'xcshareddata/xcschemes';scheme.mkdir(parents=True,exist_ok=True)
bref=f'<BuildableReference BuildableIdentifier="primary" BlueprintIdentifier="{target}" BuildableName="SynthMavenEditor.app" BlueprintName="SynthMavenEditor" ReferencedContainer="container:SynthMavenEditor.xcodeproj"/>'
(scheme/'SynthMavenEditor.xcscheme').write_text(f'''<?xml version="1.0" encoding="UTF-8"?><Scheme LastUpgradeVersion="1600" version="1.3"><BuildAction parallelizeBuildables="YES" buildImplicitDependencies="YES"><BuildActionEntries><BuildActionEntry buildForTesting="YES" buildForRunning="YES" buildForProfiling="YES" buildForArchiving="YES" buildForAnalyzing="YES">{bref}</BuildActionEntry></BuildActionEntries></BuildAction><LaunchAction buildConfiguration="Debug" selectedDebuggerIdentifier="Xcode.DebuggerFoundation.Debugger.LLDB" selectedLauncherIdentifier="Xcode.IDEFoundation.Launcher.LLDB" launchStyle="0" useCustomWorkingDirectory="NO" ignoresPersistentStateOnLaunch="NO" debugDocumentVersioning="YES" allowLocationSimulation="YES"><BuildableProductRunnable runnableDebuggingMode="0">{bref}</BuildableProductRunnable></LaunchAction><ProfileAction buildConfiguration="Release"/><AnalyzeAction buildConfiguration="Debug"/><ArchiveAction buildConfiguration="Release" revealArchiveInOrganizer="YES"/></Scheme>''')
info=dict(CFBundleDisplayName='SM FM1',CFBundleIdentifier='$(PRODUCT_BUNDLE_IDENTIFIER)',CFBundleExecutable='$(EXECUTABLE_NAME)',CFBundleName='$(PRODUCT_NAME)',CFBundlePackageType='APPL',CFBundleShortVersionString='$(MARKETING_VERSION)',CFBundleVersion='$(CURRENT_PROJECT_VERSION)',LSRequiresIPhoneOS=True,UIApplicationSceneManifest={'UIApplicationSupportsMultipleScenes':False},UILaunchScreen={},NSBluetoothAlwaysUsageDescription='Connect to your FM-1 or a compatible Bluetooth MIDI bridge to edit sounds and programs.',UISupportedInterfaceOrientations=['UIInterfaceOrientationLandscapeLeft','UIInterfaceOrientationLandscapeRight'],UIRequiresFullScreen=True,UIFileSharingEnabled=True,LSSupportsOpeningDocumentsInPlace=True)
(root/'SynthMavenEditor/Info.plist').write_bytes(plistlib.dumps(info))
