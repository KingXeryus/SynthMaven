# SPDX-License-Identifier: MPL-2.0
# Adapted from fm1-base tools/build.py, Charles Vestal, upstream fa7b203e.
# Only the BLE central/peripheral SDK build path is retained. Licences in firmware/vendor/fm1-base.
from pathlib import Path
import os, shutil
CPU='r3'
BLE=True
WIFI=False
CONF={'ble_central':True,'ble_heap_kb':20}
WIFI_INC=[]
WIFI_LIBS=[]
RADIO_LIBS=[]
def inc(*dirs):
    return ['-I'+str(Path(d).resolve().relative_to(ROOT)) for d in dirs]

def configure(root,out,compiler):
    global ROOT,SRC,OUT,tc
    ROOT=root
    SRC=root/'firmware/vendor/fm1-base'
    OUT=out
    tc=compiler

BLE_LIBS = ["btctrler.a", "btstack.a", "crypto_toolbox_Osize.a", "lib_ccm_aes.a", "wl_rf_common.a"]

BLE_SDK_SRC = ["apps/common/config/bt_profile_config.c",
               "apps/common/config/log_config/lib_btctrler_config.c",
               "apps/common/config/log_config/lib_btstack_config.c"]

BLE_INC = ["include_lib", "include_lib/driver", "include_lib/driver/device", "include_lib/driver/cpu/wl82",
           "include_lib/system", "include_lib/system/generic", "include_lib/btctrler",
           "include_lib/btctrler/port/wl82", "include_lib/update", "include_lib/btstack/third_party/common",
           "include_lib/btstack/third_party/rcsp", "include_lib/utils", "include_lib/utils/syscfg",
           "include_lib/utils/event", "include_lib/media", "include_lib/media/cpu/wl82", "apps",
           "apps/demo/demo_ble/include", "apps/common", "apps/common/include", "apps/common/config/include",
           "include_lib/btstack", "include_lib/net", "apps/common/ble/include", "include_lib/newlib/include"]

def ble_config(sdk, dst):
    """the SDK's configuration sources into dst, as BLE_CONFIG cuts them (ble_trim); written only when changed"""
    import re as _re
    import shutil
    # Keep LE scan/master/GATT/security intact. Remove only SDK RF certification
    # tests and unused Bluetooth Classic profile features from this BLE product.
    cuts = {"apps/common/config/log_config/lib_btctrler_config.c": [
        (r"(const int " + name + r"\s*=\s*)1", r"\g<1>0") for name in (
            "CONFIG_TEST_DUT_CODE", "CONFIG_TEST_FCC_CODE", "CONFIG_LMP_NAME_REQ_ENABLE",
            "CONFIG_LMP_PASSKEY_ENABLE", "CONFIG_LMP_MASTER_ESCO_ENABLE", "CONFIG_BREDR_INQUIRY",
            "config_btctler_dual_a2dp", "config_multilink_support_esco_enalbe")]}
    for f in BLE_SDK_SRC:
        text = (sdk / f).read_text(errors="ignore")
        for rx, rep in cuts.get(f, []):
            text, n = _re.subn(rx, rep, text)
            if not n:
                raise SystemExit(f"build: {f} in the SDK has changed: no {rx!r}")
        if not (dst / f).exists() or (dst / f).read_text(errors="ignore") != text:
            (dst / f).parent.mkdir(parents=True, exist_ok=True)
            (dst / f).write_text(text)

BLE_CF = ["-integrated-as", f"-mcpu={CPU}", "-mfprev1", "-fno-common", "-Oz", "-flto", "-fallow-pointer-null",
          "-fprefer-gnu-section", "-femulated-tls", "-fms-extensions", "-w", "-DSUPPORT_MS_EXTENSIONS",
          "-DCONFIG_RELEASE_ENABLE", "-DCONFIG_CPU_WL82", "-DCONFIG_FREE_RTOS_ENABLE", "-DCONFIG_THREAD_ENABLE",
          "-D__GCC_PI32V2__", "-DCONFIG_NEW_ECC_ENABLE", "-DCONFIG_SFC_ENABLE", "-DCONFIG_NO_SDRAM_ENABLE",
          "-DCONFIG_BT_ENABLE=1", "-mllvm", "-pi32v2-large-program=true"]

def radio_units():
    """copy what the Bluetooth or WiFi build needs from the SDK into build/ble (the container sees only this tree);
    return the compile commands, the objects and the link's libraries"""
    import shutil
    sdk = Path(os.environ.get("AC79_SDK", Path.home() / "fw-AC79_AIoT_SDK"))
    dst = OUT / "ble" / "sdk"
    if not (dst / ".copied").exists():
        for d in ("include_lib", "apps/common/include", "apps/common/config/include", "apps/common/ble/include",
                  "apps/demo/demo_ble/include"):
            shutil.copytree(sdk / d, dst / d, dirs_exist_ok=True,
                            ignore=lambda dd, names: [n for n in names if (Path(dd) / n).is_file()
                                                      and not n.endswith((".h", ".hpp"))])
        (dst / "lib").mkdir(exist_ok=True)
        for f in BLE_LIBS:
            shutil.copy(sdk / "cpu/wl82/liba" / f, dst / "lib" / f)
        shutil.copy(sdk / "include_lib/newlib/pi32v2-lib/libcompiler_rt.a", dst / "lib")
        (dst / ".copied").write_text("")
    ble_config(sdk, dst)
    if WIFI and not (dst / ".copied_wifi").exists():
        for d in LWIP_DIRS:                       # lwIP's sources
            for c in (sdk / "lib/net/lwip_2_1_3" / d).glob("*.c"):
                if c.name not in LWIP_SKIP:
                    (dst / "lwip" / d).mkdir(parents=True, exist_ok=True)
                    shutil.copy(c, dst / "lwip" / d / c.name)
        for f in WIFI_LIBS:
            shutil.copy(sdk / "cpu/wl82/liba" / f, dst / "lib" / f)
        (dst / ".copied_wifi").write_text("")
    flags = [*inc(*(dst / d for d in BLE_INC + WIFI_INC * WIFI)), *inc(SRC / "shim")]
    if WIFI:
        flags += ["-DSHIM_WIFI=1"]
    if CONF.get("ble_stack_kb"):                  # the Bluetooth tasks' stacks, KB each (default 8)
        flags.append(f"-DSHIM_STACK_WORDS={int(CONF['ble_stack_kb']) * 256}")
    if CONF.get("ble_heap_kb") and not WIFI:      # its heap, KB (default 24, in RAM)
        flags.append(f"-DSHIM_BLE_HEAP_KB={int(CONF['ble_heap_kb'])}u")
    o = OUT / "ble" / "o"
    o.mkdir(parents=True, exist_ok=True)
    units = [("cc", "-c", SRC / "shim" / "ctx.S", "-o", o / "ctx.o")]
    objs = [o / "ctx.o"]
    # the SDK demo's app_config.h picks its BLE demo with #defines (a -D cannot override them): the peripheral alone
    # (TRANS_DATA_EN), or with ble_central one link as a peripheral and one as a central (TRANS_MULTI_BLE_EN: the
    # controller's link count, the multi-link stack). Written from the SDK's each build, as this one wants it
    cfg = "apps/demo/demo_ble/include/app_config.h"
    text = (sdk / cfg).read_text(errors="ignore")
    if BLE and CONF.get("ble_central"):
        flags.append("-DBLE_MIDI_CENTRAL=1")
        for old, new in (("#define TRANS_DATA_EN                             1",
                          "#define TRANS_DATA_EN                             0"),
                         ("#define TRANS_MULTI_BLE_EN                        0",
                          "#define TRANS_MULTI_BLE_EN                        1"),
                         ("#define TRANS_MULTI_BLE_SLAVE_NUMS                2",
                          "#define TRANS_MULTI_BLE_SLAVE_NUMS                1"),
                         ("#define TRANS_MULTI_BLE_MASTER_NUMS               2",
                          "#define TRANS_MULTI_BLE_MASTER_NUMS               1")):
            if old not in text:
                raise SystemExit(f"build: {cfg} in the SDK has changed: no {old!r}")
            text = text.replace(old, new)
    if (dst / cfg).read_text(errors="ignore") != text:
        (dst / cfg).write_text(text)
        for f in o.glob("*.o"):
            f.unlink()                            # (built for the other configuration)
    ble_only = [*(SRC / "midi").glob("*.c"), *[dst / s for s in BLE_SDK_SRC]] if BLE else []
    for f in [*(SRC / "shim").glob("*.c"), *ble_only]:
        units.append(("cc", *BLE_CF, *flags, "-c", f, "-o", o / (f.stem + ".o")))
        objs.append(o / (f.stem + ".o"))
    if WIFI:
        lwip_inc = [*inc(SRC / "ble" / "lwip"), *flags, *inc(*(dst / d for d in LWIP_INC))]
        for d in LWIP_DIRS:
            for f in sorted((dst / "lwip" / d).glob("*.c")):
                units.append(("cc", *BLE_CF, *LWIP_CF, *lwip_inc, "-c", f, "-o", o / ("lwip_" + f.stem + ".o")))
                objs.append(o / ("lwip_" + f.stem + ".o"))
        for f in (SRC / "ble" / "wifi").glob("*.c"):
            units.append(("cc", *BLE_CF, *LWIP_CF, *lwip_inc, "-c", f, "-o", o / ("wifi_" + f.stem + ".o")))
            objs.append(o / ("wifi_" + f.stem + ".o"))
        app_inc = inc(APP, SRC / "include", GEN, *(APP / d for d in CONF.get("include", []) + CONF.get("device_include", [])
                                                     + CONF.get("net_include", [])))
        for u in CONF.get("net", []):            # the app's network code: as the SDK builds code that talks lwIP
            for f in (sorted(APP.glob(u)) if any(ch in u for ch in "*?[") else [APP / u]):
                name = "net_" + "_".join(f.relative_to(APP).with_suffix("").parts) + ".o"
                units.append(("cc", *BLE_CF, *LWIP_CF, *CONF.get("net_cflags", []), *lwip_inc, *app_inc, "-c", f,
                              "-o", o / name))
                objs.append(o / name)
    linked = (BLE_LIBS if BLE else RADIO_LIBS) + WIFI_LIBS * WIFI
    if BLE and CONF.get("ble_trim") and CONF.get("ble_central"):
        raise SystemExit("build: ble_trim cuts the central's scan, connecting and GATT client: not with ble_central")
    trimmed = ble_trim(dst)
    libs = ["--start-group", *[trimmed.get(f, dst / "lib" / f) for f in linked], dst / "lib" / "libcompiler_rt.a",
            "--end-group"]
    return units, objs, libs

# Central-safe trimming only: no scan, GATT, connection, bonding or encryption
# routines are replaced. These are Bluetooth Classic ACL and Apple notification
# service entry points unused by BLE MIDI; adapted from fm1-base's verified cuts.
BLE_TRIM = {
 ("btctrler.a", "bredr_link.c.o"): "int link_conn_tx_bulk_avaiable(void) { return 0; }",
 ("btstack.a", "ancs_client.c.o"): "void ancs_client_btstack_init_for_check(void) {}",
}
def ble_trim(dst):
    """OUT/ble/trim/LIB: the libraries with BLE_TRIM's functions replaced (cached by their stubs); {LIB: path}"""
    import shutil
    trim = OUT / "ble" / "trim"
    pick = os.environ.get("FM1B_STUBS")           # (a bisection: only these members' stubs, e.g. "ll_scan,gatt_client")
    items = {k: v for k, v in BLE_TRIM.items() if not pick or k[1].replace(".c.o", "") in pick.split(",")}
    key = repr(sorted(items.items()))
    libs = {lib: trim / lib for lib, _ in items}
    if (trim / "key").exists() and (trim / "key").read_text() == key:
        return libs
    if trim.exists():
        shutil.rmtree(trim)
    trim.mkdir(parents=True)
    for (lib, member), src in items.items():
        if not (trim / lib).exists():
            shutil.copy(dst / "lib" / lib, trim / lib)
        w = trim / member.replace(".c.o", "")
        w.mkdir()
        (w / "stub.c").write_text(src + "\n")
        tc("common/bin/llvm-ar", "x", trim / lib, member)         # (into ROOT, the tools' cwd: moved below)
        shutil.move(ROOT / member, w / member)
        tc("cc", "-mcpu=r3", "-mfprev1", "-Oz", "-flto", "-c", w / "stub.c", "-o", w / "stub.bc")
        tc("common/bin/llvm-link", w / member, f"-override={(w / 'stub.bc').resolve().relative_to(ROOT)}",
           "-o", w / "out.bc")
        shutil.copy(w / "out.bc", w / member)
        tc("common/bin/llvm-ar", "r", trim / lib, w / member)
    (trim / "key").write_text(key)
    return libs
