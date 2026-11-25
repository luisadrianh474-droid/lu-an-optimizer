# Lu-An Optimizer

![Lu-An Gaming Optimization](logo.jpg)

**Lu-An Optimizer** is a free, open-source gaming optimizer for Windows 7 / 8 / 10 / 11.
A single portable `.exe` — no install, no Python, no .NET.

## Features

- **Timer Resolution** — change the Windows timer from 15.6 ms down to 0.5 ms
  (`NtSetTimerResolution`), with presets and profiles (Gaming, Escritura,
  Accesibilidad, Normal).
- **FilterKeys** — BounceKeys, SlowKeys and RepeatKeys profiles, following
  Microsoft's rule that BounceKeys and Slow/RepeatKeys are mutually exclusive.
- **DNS Jumper** — pick a network adapter, see the current DNS, and apply
  presets: Google, Cloudflare, Quad9, OpenDNS, AdGuard, or Automatic (DHCP).
  Includes DNS cache flush and ping test.
- **Competitive Mode (one click)** — timer 0.5 ms, FilterKeys off, DNS cache
  flush, high-performance power plan, Game Mode, and a fastest-DNS test,
  with a one-click restore to normal.
- **Live ping** — current ping, average, jitter and packet loss, plus a
  test-all-DNS feature that recommends the fastest DNS for you.
- **SOCD Cleaner** — for WASD and arrow keys, Last-key-wins or Neutral mode.
  Off by default.
- **System optimizations** — disable mouse acceleration, disable fullscreen
  optimizations, close background apps (with confirmation).
- **Standby memory cleaning (ISLC-style)** — purge the standby memory list.

## ⚠️ SOCD Cleaner warning

The SOCD Cleaner is **off by default**. When you enable it, the program shows
a warning you must accept: simultaneous opposing cardinal directions (SOCD)
are **banned in some games**. Valve banned it in CS2 in 2024 and players can
be kicked or banned. Use it only in games where it is allowed, at your own
responsibility. Some anti-cheats may also detect injected input.

## Build

Requirements: Zig 0.15.1.

```sh
./build.sh
```

This produces `LuAnOptimizer.exe` (x64, GUI subsystem). The build embeds the
icon (`luan.ico`) and the logo (`logo.jpg`, via `logo_data.h`) into the
executable. See `build.sh` for the exact commands (`zig rc` + `zig cc`).

On Windows, some features (changing DNS, power plan, standby memory) need
**Run as administrator**.

## Windows Defender / SmartScreen

This program is unsigned while it is being published. Windows Defender may
flag it as a false positive: a low-level keyboard hook (`WH_KEYBOARD_LL`) plus
`SendInput` looks like a keylogger to heuristics, and the program also changes
DNS, power plan, registry and processes. The source code in this repository is
the whole program — there is no hidden code and nothing is sent anywhere
except ICMP pings to DNS servers you choose.

Releases are code-signed for free by **SignPath Foundation** (see
`.github/workflows/build.yml`). The publisher shown by Windows is therefore
"SignPath Foundation", not "Lu-An".

## License

MIT — see [LICENSE](LICENSE).
