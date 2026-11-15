# Build Polybius

The development targets are Linux x86-64 and Android ARM64. The supported
input is Polybius CUSA08328 v1.03. See the current [port status](polybius-port-status.md).

## Clone

```bash
git clone https://github.com/CrownParkComputing/polybius-recomp.git
cd polybius-recomp
```

Renderer dependencies under `gpu/third_party/` are vendored, including their
licences. No Git submodule setup is needed. Game files, prepared images, drivers,
Android signing keys, saves and build outputs stay outside Git.

## Linux prerequisites

Use a Linux x86-64 host with Clang supporting C++23, CMake 3.24+, Ninja, Git,
Python 3.10+, pkg-config, make, curl, xz and nasm. The renderer needs Vulkan
headers including Vulkan-Hpp, Vulkan loader, glslang, SDL3, fmt, Boost, robin-map,
xxHash, Zydis, zlib, X11 and Vulkan Memory Allocator headers.

On Ubuntu 24.04 the same installer used by CI installs system packages and builds
the missing SDL3/Vulkan headers. It uses sudo and installs into `/usr/local`:

```bash
bash .github/scripts/ci-deps.sh
export CC=clang-18 CXX=clang++-18
```

On another distribution install the equivalent development packages first.
A Vulkan-capable GPU/driver and desktop session are needed to play; source
compilation and most runtime checks work without a game or display.

## Build and check Linux

```bash
bash build.sh
```

Outputs are `out/polybius` and the inherited `out/tempest` runtime name. The
`out/tempest-setup` helper supplies the extractor's native XTS operation. The
build links minimal FFmpeg 9.0.2, unpacking the inherited Linux archive when its
options match, or downloading hash-checked source otherwise. Set
`TEMPEST_FFMPEG_PREBUILT=0` to build FFmpeg from source.

```bash
bash build.sh --test
(cd tests && python3 -m unittest discover -p 'test_*.py')
```

Tests requiring a missing game, graphics device or optional dependency may skip.

## Supply your game

Use your own decrypted CUSA08328 v1.03 dump under ignored `game/`:

```text
game/eboot.bin
game/sce_module/libc.prx
game/sce_module/libSceFios2.prx
game/sce_sys/param.sfo
game/assets/NeonResources.xml
game/assets/Polybius4.bnk
```

Extract a matching base and update separately, then overlay update files onto a
copy of the base dump. Keep the original packages and extraction folders.
The supported pair is CUSA08328 base01.00 plus update01.03 with content ID
`UP4461-CUSA08328_00-LLAMASOFTPLB0002`:

```bash
python3 scripts/pkg_extract.py /path/to/base.pkg --out out/base --xts-tool out/tempest-setup
python3 scripts/pkg_extract.py /path/to/update.pkg --out out/update --xts-tool out/tempest-setup
cp -a out/base/. game/
cp -a out/update/. game/
```

Only overlay an update after checking its title/content ID matches the base.
The current source does not redistribute game data or automate package selection.

The verified executable SHA-256 is:
`e02a60780769e1c743ce69870b0580b48a43c6f5a5ee870900ffe517bdb039fb`.
Other builds require investigation; `--any-build` is not a compatibility promise.
The original dump is never rewritten. The runtime aliases the executable's
`/app0/Assets` paths to the package's lower-case `assets` directory when needed.

## Prepare and play

```bash
python3 run.py prepare --game game
./play-polybius.sh
```

Use `--game /path/to/dump`, `--user /path/to/profile` or `--timeout 60` as needed.
Prepared images stay under `out/game/`. Saves are under
`user/savedata/1/CUSA08328/SAVEDATA00/`; Android has a separate profile.
Caches default to `~/.cache/polybius/` and `out/shaders/`.

The shared environment names remain `TEMPEST_*`. Polybius uses its own frame clock
and aspect ratio; the fixed addresses from earlier games are disabled.
Both draw pipelining and speculative draw preparation default to off on Linux
and Android, carrying the safe profile established by Akka Arrh. Explicit `TEMPEST_DRAW_PIPE`
and `TEMPEST_PREP_WORKERS` overrides are available for diagnostics.
L3 + R3 opens Settings; F10 and holding Select for two seconds also work.

## First start

Use the left stick to move the game's cursor. Confirm the left-hand sign with
Cross/A, wait for its detail page, then confirm again. Repeat for the right-hand
sign and final notice. Wait a few seconds between confirmation steps; the game
has a built-in input delay after each transition. These are the original game
screens. Keyboard WASD moves the stick and Space/Enter confirms.

## Android

Follow [android/README.md](../android/README.md) for dependencies, ARM64/FEX,
Turnip packaging, signing, installation and hardware checks. A fresh checkout
can build without paths to another port's workspace.

## Automation and release scope

CI builds Linux and runs runtime, Python and driver-import checks on main/PRs.
Game files, original launcher artwork, driver ZIPs, signing keys and prepared
images remain local. The inherited GUI setup and release packager are not the
Polybius distribution flow. Historical notes are retained under `docs/legacy/`.
See [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md) for source origins.
