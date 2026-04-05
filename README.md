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
4. The binary lands at `build/dgbminer.exe`.

### Build from the command line

From a Developer PowerShell / `x64 Native Tools Command Prompt`:

```powershell
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The resulting `build/dgbminer.exe` is a self-contained ~6 MB executable (statically links libcurl, jansson, OpenSSL, pthread).

---

## Solo mining (run node + miner on the same PC)

Run `dgbminer.exe` against your local `digibyted` RPC:

### Solo — sha256d
```powershell
.\dgbminer.exe -a sha256d -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — scrypt
```powershell
.\dgbminer.exe -a scrypt -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — skein
```powershell
.\dgbminer.exe -a skein -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — qubit
```powershell
.\dgbminer.exe -a qubit -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

### Solo — odo (odocrypt)
```powershell
.\dgbminer.exe -a odo -o http://127.0.0.1:14022/ --userpass=user:pass --no-getwork --no-stratum --coinbase-addr=dgb1q66lmtmlkswlphp5j7fgvg4nar4y8uf24hvlu89 -D
```

Replace `dgb1q...` with **your own** DigiByte payout address and `user:pass` with the RPC credentials from your `digibyte.conf`.

### Help / all options
```powershell
.\dgbminer.exe --help
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

## Notes

- Currently built with the **SSE2 baseline** — the AVX2/AVX-512 code paths in the upstream source have missing includes and are disabled. Enabling them is a future improvement.
- Binary is statically linked: no DLLs to distribute.
- Licensed under **GPL v3** (see LICENSE).
