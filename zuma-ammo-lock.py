#!/usr/bin/env python3
"""
Zuma Deluxe ammo color lock - warna di-RANDOM ulang tiap restart / ganti level.

Cara kerja:
  - Tiap kali konstruktor gun dipanggil (level baru / restart), script
    reset palet, deteksi warna ammo selama --detect detik, lalu pilih
    1 warna acak dari palet itu dan lock.

Usage:
    python zuma-ammo-lock.py                  # random tiap level/restart
    python zuma-ammo-lock.py --detect 8       # waktu deteksi palet lebih lama
    python zuma-ammo-lock.py --force-color 2  # warna tetap (tanpa random)
    python zuma-ammo-lock.py --no-find-writer

PENTING: jalankan script SEBELUM masuk level (sebelum gun dibuat),
karena deteksi level baru lewat hook konstruktor gun.

DISCLAIMER: kalau warna peluru kelihatan flickering (berkedip ganti-ganti
warna), itu NORMAL. Game masih sempat nulis warna aslinya, lalu script
nimpa lagi ke warna lock. Pas peluru ditembak, warnanya tetap warna yang
di-lock.
"""

import argparse
import time
import frida

PROCESS_NAME = "popcapgame1.exe"
GUN_CTOR_RVA     = 0x0CE5B0
GUN_VTABLE_RVA   = 0x1466E8
BALL_VTABLE_RVA  = 0x146800
GUN_CUR_OFF      = 0x2C
GUN_NXT_OFF      = 0x30
BALL_COLOR_OFF   = 0x08


def build_agent(find_writer, detect_ms, fixed_color):
    js = r"""
'use strict';

const find_writer = __FIND_WRITER__;
const DETECT_MS       = __DETECT_MS__;
const FIXED_COLOR     = __FIXED_COLOR__;   // -1 = random tiap level
const GUN_CTOR_RVA    = __GUN_CTOR_RVA__;
const GUN_VTABLE_RVA  = __GUN_VTABLE_RVA__;
const BALL_VTABLE_RVA = __BALL_VTABLE_RVA__;
const GUN_CUR_OFF     = __GUN_CUR_OFF__;
const GUN_NXT_OFF     = __GUN_NXT_OFF__;
const BALL_COLOR_OFF  = __BALL_COLOR_OFF__;

let moduleBase, moduleEnd, gunVtAddr, ballVtAddr;
let gunObjects = new Map();
let ammo = [];
let mode = 'detect';
let forcedColor = -1;
let observedColors = new Set();
let statWrites = 0;
let watchpointActive = false;
let watchpointThread = null;
let watchpointBall = null;
let foundWriterRva = null;
let levelToken = 0;
let lastLevelStart = 0;

function say(m) { send({type:'log', msg:m}); }

function isBall(p) {
  if (!p || p.isNull()) return false;
  try { const vt = p.readPointer();
        return !vt.isNull() && vt.equals(ballVtAddr); }
  catch(_) { return false; }
}
function isGun(p) {
  if (!p || p.isNull()) return false;
  try { const vt = p.readPointer();
        return !vt.isNull() && vt.equals(gunVtAddr); }
  catch(_) { return false; }
}

function forceNow() {
  if (mode !== 'lock' || forcedColor < 0) return;
  for (const b of ammo) {
    try {
      const vt = b.readPointer();
      if (!vt || vt.isNull() || !vt.equals(ballVtAddr)) continue;
      const c = b.add(BALL_COLOR_OFF).readU32();
      if (c >= 64) continue;
      if (c === forcedColor) continue;
      b.add(BALL_COLOR_OFF).writeU32(forcedColor);
      statWrites++;
    } catch(_) {}
  }
}

function refreshAmmo() {
  const seen = new Set(), next = [];
  for (const [k, gun] of gunObjects) {
    if (!isGun(gun)) { gunObjects.delete(k); continue; }
    for (const off of [GUN_CUR_OFF, GUN_NXT_OFF]) {
      try {
        const p = gun.add(off).readPointer();
        if (isBall(p)) {
          const pk = p.toString();
          if (!seen.has(pk)) { seen.add(pk); next.push(p); }
        }
      } catch(_) {}
    }
  }
  ammo = next;
  if (mode === 'detect') {
    for (const b of ammo) {
      try {
        const c = b.add(BALL_COLOR_OFF).readU32();
        if (c < 64 && !observedColors.has(c)) {
          observedColors.add(c);
          send({type:'observe', color:c, total:observedColors.size});
        }
      } catch(_) {}
    }
  }
}

// ===== LEVEL BARU / RESTART: reset lalu pilih warna acak =====
function lockColor(c, palette) {
  forcedColor = c;
  mode = 'lock';
  send({type:'locked', color:c, palette:palette});
  refreshAmmo();
  forceNow();
}

function finishDetect(token) {
  if (token !== levelToken) return;          // sudah ada level lain
  refreshAmmo();
  const palette = Array.from(observedColors).sort((a, b) => a - b);
  if (palette.length === 0) {                // belum ada ammo, tunggu lagi
    setTimeout(() => finishDetect(token), 500);
    return;
  }
  const chosen = palette[Math.floor(Math.random() * palette.length)];
  lockColor(chosen, palette);
}

function beginLevel() {
  const now = Date.now();
  if (now - lastLevelStart < 1500) return;   // debounce
  lastLevelStart = now;
  levelToken++;
  const token = levelToken;
  observedColors.clear();
  if (FIXED_COLOR >= 0) {
    send({type:'newlevel', fixed:true});
    lockColor(FIXED_COLOR, [FIXED_COLOR]);
    return;
  }
  mode = 'detect';
  forcedColor = -1;
  send({type:'newlevel', fixed:false});
  setTimeout(() => finishDetect(token), DETECT_MS);
}

// ===== HARDWARE WATCHPOINT WRITER FINDER =====
function startWatchpoint() {
  if (!find_writer) return;
  if (watchpointActive) return;
  if (ammo.length === 0) { say('watchpoint: no ammo yet'); return; }

  const ball = ammo[0];
  const addr = ball.add(BALL_COLOR_OFF);
  watchpointBall = ball;

  const threads = Process.enumerateThreads();
  if (threads.length === 0) { say('no threads'); return; }
  const tid = threads[0].id;
  watchpointThread = threads[0];

  Process.setExceptionHandler(e => {
    if (!watchpointActive) return false;
    if (!['breakpoint', 'single-step'].includes(e.type)) return false;
    const pc = e.context.pc;
    const mod = Process.findModuleByAddress(pc);
    let rva = '?';
    if (mod) rva = '0x' + pc.sub(mod.base).toString(16);
    send({type:'writer-found', rva:rva, pc:pc.toString(), module:mod?mod.name:'?'});
    foundWriterRva = rva;
    try { watchpointThread.unsetHardwareWatchpoint(0); } catch(_) {}
    watchpointActive = false;
    return true;
  });

  try {
    watchpointThread.setHardwareWatchpoint(0, addr, 4, 'w');
    watchpointActive = true;
    say('watchpoint set on ' + addr + ' (tid=' + tid + ')');
    setTimeout(() => {
      if (watchpointActive) {
        try { watchpointThread.unsetHardwareWatchpoint(0); } catch(_) {}
        watchpointActive = false;
        say('watchpoint timeout, no write detected');
      }
    }, 3000);
  } catch(e) {
    say('watchpoint set failed: ' + e);
  }
}

// ===== HOOK WRITER =====
function hookWriter(writerRva) {
  const rva = typeof writerRva === 'string' ? parseInt(writerRva, 16) : writerRva;
  const target = moduleBase.add(rva);
  let insn;
  try { insn = Instruction.parse(target); } catch(e) {
    say('parse fail: ' + e); return false;
  }
  if (insn.mnemonic !== 'mov') {
    say('not mov: ' + insn.mnemonic + ' @ ' + insn.opStr);
    return false;
  }
  const dst = insn.operands[0], src = insn.operands[1];
  if (!dst.mem || !dst.mem.base) {
    say('dst not [reg..]: ' + insn.opStr); return false;
  }
  const baseReg = dst.mem.base;
  const disp = Number(dst.mem.disp) || 0;

  function resolveBall(ctx) {
    try {
      const b = ctx[baseReg];
      if (!b || b.isNull()) return null;
      let ball = b;
      if (disp === BALL_COLOR_OFF) ball = b;
      else if (disp === 0) ball = b.sub(BALL_COLOR_OFF);
      else return null;
      return isBall(ball) ? ball : null;
    } catch(_) { return null; }
  }

  if (src.type === 'reg') {
    const srcReg = src.value;
    Interceptor.attach(target, {
      onEnter() {
        if (mode !== 'lock' || forcedColor < 0) return;
        if (!resolveBall(this.context)) return;
        this.context[srcReg] = forcedColor;
        statWrites++;
      }
    });
    say('Hooked writer ' + insn.opStr + ' @ rva 0x' + rva.toString(16));
    return true;
  } else if (src.type === 'imm') {
    Interceptor.attach(target, {
      onEnter() {
        if (mode !== 'lock' || forcedColor < 0) { this.ball = null; return; }
        this.ball = resolveBall(this.context);
      },
      onLeave() {
        if (!this.ball) return;
        try { this.ball.add(BALL_COLOR_OFF).writeU32(forcedColor); statWrites++; }
        catch(_) {}
      }
    });
    say('Hooked writer (imm) ' + insn.opStr + ' @ rva 0x' + rva.toString(16));
    return true;
  }
  say('unsupported src ' + src.type + ' for ' + insn.opStr);
  return false;
}

// ===== INIT =====
const mod = Process.findModuleByName('popcapgame1.exe');
if (!mod) { send({type:'error', message:'module not found'}); }
else {
  moduleBase = mod.base;
  moduleEnd = mod.base.add(mod.size);
  gunVtAddr = moduleBase.add(GUN_VTABLE_RVA);
  ballVtAddr = moduleBase.add(BALL_VTABLE_RVA);
  say('base=' + moduleBase);

  try {
    Interceptor.attach(moduleBase.add(GUN_CTOR_RVA), {
      onEnter(args) { this.obj = this.context.ecx; },
      onLeave() {
        if (!this.obj || this.obj.isNull()) return;
        if (!isGun(this.obj)) return;
        const k = this.obj.toString();
        if (!gunObjects.has(k)) {
          gunObjects.set(k, this.obj);
          send({type:'gun', address:k, total:gunObjects.size});
        }
        // dipanggil tiap level baru / restart -> random ulang
        beginLevel();
      }
    });
    say('ctor hook installed');
  } catch(e) { send({type:'error', message:'ctor: '+e}); }
}

recv('startWatchpoint', function() { startWatchpoint(); });
recv('hookWriter', function(msg) {
  const ok = hookWriter(msg.rva);
  send({type:'writer-hooked', rva: msg.rva, ok: ok});
});

setInterval(refreshAmmo, 250);
setInterval(forceNow, 1);
setInterval(function() {
  send({type:'stats', guns:gunObjects.size, ammo:ammo.length, writes:statWrites});
  statWrites = 0;
}, 2000);
"""
    return (js
            .replace('__FIND_WRITER__', 'true' if find_writer else 'false')
            .replace('__DETECT_MS__', str(int(detect_ms)))
            .replace('__FIXED_COLOR__', str(-1 if fixed_color is None else fixed_color))
            .replace('__GUN_CTOR_RVA__', hex(GUN_CTOR_RVA))
            .replace('__GUN_VTABLE_RVA__', hex(GUN_VTABLE_RVA))
            .replace('__BALL_VTABLE_RVA__', hex(BALL_VTABLE_RVA))
            .replace('__GUN_CUR_OFF__', hex(GUN_CUR_OFF))
            .replace('__GUN_NXT_OFF__', hex(GUN_NXT_OFF))
            .replace('__BALL_COLOR_OFF__', hex(BALL_COLOR_OFF)))


def _shutdown(script, session):
    try: script.unload()
    except Exception: pass
    try: session.detach()
    except Exception: pass


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('--detect', type=float, default=6.0,
                    help='detik kumpulin palet warna tiap level baru')
    ap.add_argument('--wait-ammo', type=float, default=120.0)
    ap.add_argument('--force-color', type=int, default=None,
                    help='warna tetap, tanpa random')
    ap.add_argument('--verbose', action='store_true')
    ap.add_argument('--no-find-writer', action='store_true')
    args = ap.parse_args()

    try:
        import frida  # noqa
    except ImportError:
        print("Frida belum terpasang. Jalankan: pip install frida")
        return 1

    try:
        session = frida.get_local_device().attach(PROCESS_NAME)
    except Exception as exc:
        print(f"Gagal attach: {exc}")
        return 1

    script = session.create_script(
        build_agent(not args.no_find_writer,
                    args.detect * 1000, args.force_color))

    stats = {}
    state = {'locked_once': False, 'locked': False}

    def on_message(message, data):
        if message.get('type') == 'send':
            p = message.get('payload', {})
            k = p.get('type')
            if k == 'observe':
                print(f"[PALET] +warna {p['color']}  (total: {p['total']})")
            elif k == 'gun':
                print(f"[GUN] #{p['total']} di {p['address']}")
            elif k == 'newlevel':
                if p.get('fixed'):
                    print("[LEVEL] level baru/restart (warna tetap).")
                else:
                    print(f"[LEVEL] level baru/restart! Deteksi palet "
                          f"{args.detect:.1f}s...")
            elif k == 'locked':
                print(f"[LOCK] palet={p['palette']} -> warna acak {p['color']}")
                state['locked'] = True
            elif k == 'log':
                print(f"[INFO] {p['msg']}")
            elif k == 'stats':
                stats.update(p)
                if args.verbose:
                    print(f"[STATS] guns={p['guns']} ammo={p['ammo']} "
                          f"writes={p['writes']}")
            elif k == 'writer-found':
                print(f"[WRITER] rva={p['rva']} pc={p['pc']} module={p['module']}")
                print(f"[HOOK] memasang hook writer rva={p['rva']}")
                script.post({'type': 'hookWriter', 'rva': p['rva']})
            elif k == 'writer-hooked':
                print(f"[HOOK] writer rva={p['rva']} -> {p['ok']}")
            elif k == 'error':
                print(f"[ERROR] {p.get('message')}")
        elif message.get('type') == 'error':
            print("[FRIDA ERROR]", message.get('stack', message))

    script.on('message', on_message)
    script.load()

    print("Siap. Masuk level / restart / ganti level -> warna di-random ulang.")
    print("Catatan: peluru kelihatan flickering itu normal, "
          "pas ditembak tetap warna yang di-lock.")
    print("Ctrl+C untuk berhenti.")

    try:
        t0 = time.time()
        while not state['locked']:
            time.sleep(0.2)
            if time.time() - t0 > args.wait_ammo:
                print("[GAGAL] belum ada level yang terdeteksi. "
                      "Pastikan script jalan SEBELUM masuk level.")
                _shutdown(script, session)
                return 1

        # watchpoint writer finder cuma sekali, setelah lock pertama
        if not args.no_find_writer and not state['locked_once']:
            state['locked_once'] = True
            time.sleep(3)
            print("[WATCHPOINT] aktif 3s - tembak/geser biar game re-roll warna.")
            script.post({'type': 'startWatchpoint'})

        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nMenghentikan...")
    finally:
        _shutdown(script, session)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())