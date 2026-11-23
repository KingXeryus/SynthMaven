// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Kerem Kilic (Ellic Studio)
// Portions Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
//
// The FM-1 USB-MIDI update protocol over WebMIDI (after tools/fm1_install.py).
// Step 1: the running firmware (official
// or Melodee) reads parts of the package and stages the update loader. Step 2:
// the loader reads the whole image and writes it. Both steps are "the device
// asks, we answer": cmd 0x30 read requests on the logical image.

const HS_QUERY = [0xF0, 0x00, 0x32, 0x45, 0x00, 0x00, 0x00, 0x40, 0x7F, 0xF7];
const UPGRADE = [0xF0, 0x22, 0x24, 0x35, 0x7F, 0xF7];
const MAXDATA = 512;
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const IS_OTA = (id) => /^ota-/i.test(id.model);
// the update loaders on the handshake: Melodee's own (firmware/loader/loader.c), which writes the
// app area only, and the official one, which writes the whole package including the boot data and
// reports the name in its update record (firmware/src/ota.c). Each package goes only to its own.
export const MELODEE_LOADER = "ota-FM-1_900";
export const STOCK_LOADER = "ota-FM-1_015";

// errors carry a code the pages translate: notfound, model, stopped, noloader, lost, noreturn,
// mismatch (detail: the identity the device reports), badreq, wrongloader (detail: the loader's
// identity). A step-1 error has lastRead: the last image address the running firmware read.
function fail(code, msg, detail = "") { const e = new Error(msg); e.code = code; e.detail = detail; return e; }

export function pack7(data) {
  const out = []; let acc = 0, nb = 0;
  for (const b of data) {
    acc |= b << nb; nb += 8;
    while (nb >= 7) { out.push(acc & 0x7F); acc >>>= 7; nb -= 7; }
  }
  if (nb) out.push(acc & 0x7F);
  return out;
}

export function unpack7(s) {
  const out = []; let acc = 0, nb = 0;
  for (const b of s) {
    acc |= (b & 0x7F) << nb; nb += 7;
    while (nb >= 8) { out.push(acc & 0xFF); acc >>>= 8; nb -= 8; }
  }
  return out;
}

function response(addr, data, fl = 0) {
  const len = data.length;
  const body = [0x00, 0x59, 0x30, (len + 8) & 0xFF, ((len + 8) >> 8) & 0xFF, 0, fl,
    addr & 0xFF, (addr >>> 8) & 0xFF, (addr >>> 16) & 0xFF, (addr >>> 24) & 0xFF,
    len & 0xFF, (len >> 8) & 0xFF, 0, ...data];
  let s = 0;
  for (let i = 6; i < body.length; i++) s += body[i];
  body.push(~s & 0xFF);
  return [0xF0, ...pack7(body), 0xF7];
}

function parseRequest(pkt) {
  if (pkt[0] !== 0xF0 || pkt[pkt.length - 1] !== 0xF7) return null;
  const u = unpack7(pkt.slice(1, -1));
  if (u.length !== 15 || u[0] !== 0 || u[1] !== 0x59 || u[2] !== 0x30) return null;
  let s = 0;
  for (let i = 6; i < 14; i++) s += u[i];
  if ((~s & 0xFF) !== u[14]) return null;
  return { fl: u[6], addr: (u[7] | (u[8] << 8) | (u[9] << 16) | (u[10] << 24)) >>> 0, len: u[11] | (u[12] << 8) | (u[13] << 16) };
}

function parseIdentity(pkt) {
  const d = unpack7(pkt.slice(1, -1));
  if (d.length !== 34 || d[0] !== 0 || d[1] !== 0x59 || d[2] !== 0x11) return null;
  const txt = String.fromCharCode(...d.slice(6, 33)).replace(/\0+$/, "");
  if (/^FM1_(OFFICIAL_OPEN|COMMUNITY_\d+)$/.test(txt))
    return {text:txt,model:"FM-1",open:true};
  const m = /^([^_]+)_(\d+)$/.exec(txt);
  return m ? { text: txt, model: m[1], version: parseInt(m[2], 10) } : null;
}

// one MIDI in/out pair with a SysEx queue
class Link {
  constructor(input, output) {
    this.input = input; this.output = output; this.q = []; this.waiter = null;
    input.onmidimessage = (ev) => {
      if (ev.data[0] !== 0xF0) return;
      this.q.push(Array.from(ev.data));
      if (this.waiter) { const w = this.waiter; this.waiter = null; w(); }
    };
  }
  get lost() { return this.input.state === "disconnected" || this.output.state === "disconnected"; }
  // false when the port is gone (unplugged): WebMIDI throws on a disconnected output
  send(bytes) {
    try { this.output.send(bytes); return true; } catch (_) { return false; }
  }
  drain() { this.q.length = 0; }
  async read(ms) {
    if (!this.q.length) {
      let tm;
      await Promise.race([new Promise((r) => { this.waiter = r; }), new Promise((r) => { tm = setTimeout(r, ms); })]);
      clearTimeout(tm);
      this.waiter = null;
    }
    return this.q.shift() || null;
  }
  close() { this.input.onmidimessage = null; }
}

async function handshake(link, tries = 3) {
  link.drain();
  for (let t = 0; t < tries; t++) {
    if (!link.send(HS_QUERY)) return null;
    const end = Date.now() + 1000;
    while (Date.now() < end) {
      const p = await link.read(end - Date.now());
      if (!p) break;
      const id = parseIdentity(p);
      if (id) return id;
    }
  }
  return null;
}

export class Updater {
  constructor(access, log) {
    this.access = access;
    this.log = log || (() => {});
  }

  // every in/out pair with the same port name that answers the handshake
  async find(filter) {
    const outs = [...this.access.outputs.values()];
    for (const input of this.access.inputs.values()) {
      if (input.state === "disconnected") continue;
      if (!/synthmaven|fm-1|melodee|felucca|ota|composite|sinco|usb-midi/i.test(input.name || "")) continue;   // never probe other gear
      const output = outs.find((o) => o.name === input.name && o.state !== "disconnected");
      if (!output) continue;
      try { await input.open(); await output.open(); } catch (_) { continue; }
      const link = new Link(input, output);
      const id = await handshake(link, 2);
      if (id && (!filter || filter(id))) return { link, id, name: input.name };
      link.close();
    }
    return null;
  }

  async waitFor(filter, ms) {
    const end = Date.now() + ms;
    while (Date.now() < end) {
      const dev = await this.find(filter);
      if (dev) return dev;
      await sleep(1000);
    }
    return null;
  }

  // serve read requests until the device asks for one of the finish addresses; stops early
  // when the port disappears (lost: true) instead of waiting out idleMs
  async serve(link, image, finishAddr, idleMs, onProgress) {
    let served = 0, last = Date.now(), lastRead = -1;
    for (;;) {
      const pkt = await link.read(1000);
      if (!pkt) {
        if (link.lost) return { served, finished: false, lost: true, lastRead };
        if (Date.now() - last > idleMs) return { served, finished: false, lastRead };
        continue;
      }
      const r = parseRequest(pkt);
      if (!r) continue;
      last = Date.now();
      if (r.addr === 0xE0000000 || r.addr === 0xF0000000) {
        link.send(response(r.addr, [...new TextEncoder().encode("success"), 0]));
        if (r.addr === finishAddr) return { served, finished: true };
        continue;
      }
      if (r.len > MAXDATA || r.addr + r.len > image.length) throw fail("badreq", `bad request ${r.addr.toString(16)}+${r.len}`);
      lastRead = r.addr;
      await sleep(10);
      if (!link.send(response(r.addr, Array.from(image.subarray(r.addr, r.addr + r.len)), r.fl))) {
        return { served, finished: false, lost: true, lastRead };
      }
      served++;
      if (onProgress) onProgress(served, r.addr);
    }
  }

  // full install: running firmware -> loader -> new firmware; loader: the identity of the
  // package's own loader (MELODEE_LOADER, STOCK_LOADER), any other one gets nothing
  async install(image, product, onStep, loader) {
    const step = onStep || (() => {});
    const dev = await this.find((id) => !IS_OTA(id));
    if (!dev) throw fail("notfound", "FM-1 not found (USB cable, and no other app using it?)");
    const [model] = product.split("_");
    if (dev.id.open) { dev.link.close(); throw fail("open", "Return from M-VAVE Open firmware to official firmware before installing Melodee."); }
    if (dev.id.model !== model) { dev.link.close(); throw fail("model", `the device is ${dev.id.text}, the package is for ${product}`); }
    step("start", dev.id.text);
    dev.link.send(UPGRADE);
    await sleep(2000);
    const s1 = await this.serve(dev.link, image, 0xE0000000, 8000, (n, addr, bytes) => step("verify", n, bytes));
    dev.link.close();
    if (!s1.finished) {
      const e = fail(s1.lost ? "lost" : "stopped", `the device ${s1.lost ? "was disconnected" : "stopped"} after ${s1.served} requests: nothing was written`);
      e.lastRead = s1.lastRead;
      throw e;
    }
    step("loader");
    await sleep(3000);
    const ota = await this.waitFor((id) => id.model === "ota-" + model, 30000);
    if (!ota) throw fail("noloader", "the update loader did not appear. Replug the USB cable and press Install again: the device stays in update mode until it is finished.");
    if (ota.id.text !== loader) {
      ota.link.close();
      throw fail("wrongloader", `the device started ${ota.id.text}, this package goes only to ${loader}: nothing was sent to it`, ota.id.text);
    }
    const s2 = await this.write(ota, image, step);
    if (!s2.finished) throw fail(s2.lost ? "lost" : "stopped", `the loader ${s2.lost ? "was disconnected" : "stopped"} after ${s2.served} requests. Reconnect and press Install again. With boot-protected firmware, power on holding OCT− alone if needed; three short key-light flashes mean update mode.`);
    step("reboot");
    await sleep(3000);
    const back = await this.waitFor((id) => !IS_OTA(id), 40000);
    if (!back) throw fail("noreturn", "the device did not come back: power-cycle it");
    back.link.close();
    if (back.id.text !== product) throw fail("mismatch", `installed, but the device reports ${back.id.text}`, back.id.text);
    step("done", back.id.text);
    return back.id.text;
  }

  // step 2 with the loader found by find()/waitFor()
  async write(ota, image, step) {
    ota.link.send(UPGRADE);
    await sleep(2000);
    const s2 = await this.serve(ota.link, image, 0xF0000000, 180000, (n, addr, bytes) => step("write", n, bytes));
    ota.link.close();
    return s2;
  }

  // resume: the device is already in update mode -> true when the write finished. Only the
  // package's own loader (identity `loader`) is written to; false if it is not there.
  async resume(image, onStep, product, loader) {
    const model = product && product.split("_")[0];
    const ota = await this.find((id) => id.text === loader);
    if (!ota) return false;
    const step = onStep || (() => {});
    const result = await this.write(ota, image, step);
    if (!result.finished) return false;
    if (product) {
      step("reboot");
      await sleep(3000);
      const back = await this.waitFor((id) => id.model === model, 40000);
      if (!back) throw fail("noreturn", "The FM-1 did not return after resuming the write.");
      back.link.close();
      if (back.id.text !== product) throw fail("mismatch", "The FM-1 reports a different firmware after resuming.", back.id.text);
      step("done", back.id.text);
    }
    return true;
  }

  // "Replace programs and sounds" (chosen before installing): with the new firmware running, erase every used
  // slot of the shared user library (editor command 80: op 0 list, op 5 erase; all engines, 384 slots).
  // Projects, scales, MIDI mappings and settings stay. -> the number of slots erased
  async clearLibrary(onStep) {
    const step = onStep || (() => {});
    const dev = await this.waitFor((id) => !IS_OTA(id), 40000);
    if (!dev) throw fail("notfound", "FM-1 not found after the install: erase sounds in the editor instead");
    const ask = async (args, ms = 8000) => {          // F0 7D 46 4C 80 args F7 -> the reply's args (op, slot lo, hi, rc, ...)
      dev.link.drain();
      if (!dev.link.send([0xF0, 0x7D, 0x46, 0x4C, 80, ...args, 0xF7])) throw fail("lost", "The FM-1 was disconnected while erasing sounds.");
      const end = Date.now() + ms;
      while (Date.now() < end) {
        const p = await dev.link.read(end - Date.now());
        if (p && p[1] === 0x7D && p[2] === 0x46 && p[3] === 0x4C && p[4] === 80 && p[5] === args[0] && p[6] === args[1] && p[7] === args[2]) return p.slice(5, -1);
      }
      throw fail("stopped", "The FM-1 did not answer while erasing sounds.");
    };
    let erased = 0;
    try {
      step("clear");
      for (let s = 0, total = 1; s < total;) {
        const r = await ask([0, s & 0x7F, s >> 7, 16]);   // list: rc, count, total lo/hi, per slot used engine old name\0
        if (r[3]) throw fail("stopped", `The FM-1 could not list its sounds (code ${r[3]}).`);
        const n = r[4], keep = [];
        total = r[5] | (r[6] << 7);
        for (let k = 0, i = 7; k < n; k++) {
          if (r[i] || r[i + 2]) keep.push(s + k);        // used, or an older-format record
          i += 3;
          while (i < r.length && r[i++] !== 0);
        }
        for (const k of keep) {
          const e = await ask([5, k & 0x7F, k >> 7]);
          if (e[3]) throw fail("stopped", `Sound U${k + 1} could not be erased (code ${e[3]}). Stop playback and try again in the editor.`);
          erased++;
        }
        if (!n) break;
        s += n;
      }
    } finally { dev.link.close(); }
    step("cleared", erased);
    return erased;
  }
}
