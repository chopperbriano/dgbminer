# dgbminer for Windows

A DigiByte-optimized CPU miner — **Windows x64 build**. Supports **sha256d**, **scrypt**, **skein**, **qubit**, and **odocrypt**.

Derived from [Jongjan88/dgbminer](https://github.com/Jongjan88/dgbminer) (a cpuminer-opt fork).
This fork has been ported to build with **MSVC on Windows x64** via **CMake + VSCode**.

---

## Build (Windows x64)

### Prerequisites

| Tool | Install |
|---|---|
| **Visual Studio 2022+ Build Tools** (with C++ workload) | `winget install Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"` |
| **vcpkg** (for OpenSSL) | `git clone https://github.com/microsoft/vcpkg C:\vcpkg && C:\vcpkg\bootstrap-vcpkg.bat` |
| **OpenSSL static libs** | `C:\vcpkg\vcpkg install openssl:x64-windows-static` |
| **VSCode** + **CMake Tools** + **C/C++** extensions | (VSCode will prompt via `.vscode/extensions.json`) |

CMake, Ninja, and MSVC are all included with the VS Build Tools C++ workload.

### Build in VSCode (recommended)

1. Open this folder in VSCode.
2. When prompted, select the **Visual Studio Community/BuildTools 2022 Release - amd64** kit.
3. Press **F7** (or Ctrl+Shift+P → `CMake: Build`).
4. The binary lands at `build/dgbminerwindows.exe`.

### Build from the command line

From a Developer PowerShell / `x64 Native Tools Command Prompt`:

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The resulting `build/dgbminerwindows.exe` is a self-contained ~6 MB executable (statically links libcurl, jansson, OpenSSL, pthread).

---

## Solo mining (run node + miner on the same PC)

Run `dgbminerwindows.exe` against your local `digibyted` RPC:

### Solo — sha256d
```powershell
.\dgbminerwindows.exe -a sha256d -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — scrypt
```powershell
.\dgbminerwindows.exe -a scrypt -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — skein
```powershell
.\dgbminerwindows.exe -a skein -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — qubit
```powershell
.\dgbminerwindows.exe -a qubit -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — odo (odocrypt)
```powershell
.\dgbminerwindows.exe -a odo -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

Replace `dgb1q...` with **your own** DigiByte payout address and `user:pass` with the RPC credentials from your `digibyte.conf`.

### Help / all options
```powershell
.\dgbminerwindows.exe --help
```

---

## DigiByte node config

Place this in `%APPDATA%\DigiByte\digibyte.conf` (Windows) to let the miner talk to your local node.

### Mainnet
```ini
maxconnections=300
listen=1
server=1
algo=sha256d
#algo=scrypt
#algo=skein
#algo=qubit
#algo=odo

rpcuser=user
rpcpassword=pass
rpcallowip=127.0.0.1
rpcport=14022
```

### Testnet
```ini
maxconnections=300
testnet=1
listen=1
server=1
algo=sha256d

[test]
rpcuser=user
rpcpassword=pass
rpcallowip=127.0.0.1
rpcport=14022
```

> **Security note:** Never use `rpcallowip=0.0.0.0/0` on a machine reachable from the internet. Keep RPC bound to `127.0.0.1` or a LAN subnet.

---

## Config file

Instead of passing everything on the command line, drop the flags into a
JSON file and point the miner at it with `-c`. An example config is included:

```powershell
.\dgbminerwindows.exe -c cpuminer-conf.json
```

Example `cpuminer-conf.json`:
```json
{
    "algo": "sha256d",
    "url": "http://127.0.0.1:14022/",
    "userpass": "user:pass",
    "coinbase-addr": "dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89",
    "no-getwork": true,
    "no-stratum": true,
    "threads": 4,
    "debug": false
}
```

Any option key matches the long option name (without the leading `--`).

---

## Troubleshooting

Set `DGBMINER_DEBUG=1` to have the miner write a diagnostic trace
to `dgbminer_tui.log` in the current directory:

```powershell
$env:DGBMINER_DEBUG=1; .\dgbminerwindows.exe -a sha256d ...
```

This captures TUI init state, header repaints, and every log line routed
through the scrolling region — useful if the TUI is misbehaving.

---

## Algorithm efficiency

Under the current **SSE2 baseline** build (the AVX2/AVX-512 upstream paths have
missing includes and are disabled), each algorithm registers its best available
SSE2 implementation. What you get:

| Algo       | Implementation               | Parallelism | Notes |
|------------|------------------------------|-------------|-------|
| **sha256d**| `scanhash_sha256d_4way`      | 4-way SSE2  | Optimal for SSE2. 4 nonces hashed in parallel via 128-bit SSE2 regs. |
| **scrypt** | `scanhash_scrypt` (THROUGHPUT=4) | 4-way SSE2 | Optimal for SSE2. `scrypt_core_simd128` / `scrypt_core_4way`. |
| **odo**    | `scanhash_odo`               | 1-way       | Only one implementation exists; odocrypt changes every few weeks. |
| **skein**  | `scanhash_skein`             | 1-way       | Skein's 4-way path (`skein-hash-4way.c`) requires `__m256i` (AVX2); no SSE2 parallel version exists upstream. |
| **qubit**  | `scanhash_qubit`             | 1-way       | Qubit's 2-way path requires AVX2 + AES-NI; the SSE2 fallback runs serially. |

**sha256d, scrypt, and odo are already at their best** for a CPU build.

**skein and qubit** run serially without AVX2. Enabling AVX2 (a separate
refactor to fix the missing include chains in `cubehash_sse2.c`,
`luffa_for_sse2.c`, `scrypt-core-4way.c`, etc.) would unlock:
- `scanhash_skein_4way` (4 hashes in parallel via AVX2)
- `scanhash_qubit_2way` (2 hashes in parallel via AVX2 + AES-NI)
...which would roughly 2-4× those two algos' hash rates on this CPU.

## Notes

- Currently built with the **SSE2 baseline** — the AVX2/AVX-512 code paths in the upstream source have missing includes and are disabled. Enabling them is a future improvement (see **Algorithm efficiency** table above).
- Binary is statically linked: no DLLs to distribute.
- Licensed under **GPL v3** (see LICENSE).


