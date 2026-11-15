# Polybius on Android

Target: Retroid Pocket Flip2 (Snapdragon 865, Adreno 650, Android 13).
The application ID is `com.crownparkcomputing.polybius`, activity
`.PolybiusActivity`, label `Polybius`. It installs separately from Tempest.
Game files stay outside the APK.

## Host prerequisites

Start with the Linux development dependencies in [docs/building.md](../docs/building.md).
Install a JDK 17 or newer, `zip`, Android SDK platform 33 or newer, SDK build-tools
and platform-tools, and an ARM64-capable NDK. The tested NDK is
`30.0.14904198`; the native build targets API 31. Point the variables at your SDK:

```bash
export ANDROID_SDK_ROOT="$HOME/Android/Sdk"
export ANDROID_NDK_HOME="$ANDROID_SDK_ROOT/ndk/30.0.14904198"
export PATH="$ANDROID_SDK_ROOT/platform-tools:$PATH"
```

## Prepare dependencies

```bash
bash tools/setup_android_deps.sh
source out/android-env.sh
```

This stages only Boost, robin-map, Vulkan/Vulkan-Hpp and VMA headers from the host,
builds hash-checked SDL3 3.4.16 for ARM64, checks out pinned libadrenotools and
builds minimal Android FFmpeg. Outputs remain under `out/`. For nonstandard host
header prefixes set `TEMPEST_ANDROID_HEADER_PREFIXES=/prefix/one:/prefix/two`.
It never copies the host's libc headers into the NDK include path.

## Build ARM64 runtime

```bash
bash tools/build_android_fex.sh --ndk
```

The script downloads FEX at `f2b679f6028ce1c38875233aecfcf5d3f8ebecec`,
applies `android/fexcore-only.patch`, fetches its selected submodules and builds
`out/android-fex-ndk/android-libs/libmain.so` plus the execution checks.
It refuses a checkout at another revision. `TEMPEST_BUILD_JOBS=4` reduces build
parallelism on smaller hosts.

An existing patched FEX checkout can be reused with `TEMPEST_FEX_SOURCE` and
`TEMPEST_FEX_SOURCE_READONLY=1`; this is optional. Source the generated environment
again in each new shell before rebuilding or packaging.

## Driver and signing

Use the Standard Adreno 6xx/7xx [Turnip build dated November 11, 2026](https://github.com/The412Banner/Banners-Turnip/releases/tag/v27.0.0-20261111-r2),
`Turnip-v27.0.0-20261111-r2.zip`. ZIP SHA-256:
`bcc118d8a8bb4206f2012c1941252f46eb92b3da5cc579af368ac56fb2769271`.
Its library SHA-256 is
`472c1d0b4b415229facc3015e73dd4f5c8298cdbc7207d5df8b60edfda64039e`.
Keep the ZIP under `out/` or outside the checkout.

This build contains Mesa's [KGSL zero-timeout fix](https://github.com/chaotic-cx/mesa-mirror/commit/e984ef29).
Older R5 builds can block on nonblocking completion checks and cause slow gameplay.
The driver ZIP and library are not checked into this repository;
`assets/driver-source.json` records exact APK provenance.

For local development create a debug signing key once, if you do not have one:

```bash
mkdir -p "$HOME/.android"
keytool -genkeypair -keystore "$HOME/.android/debug.keystore" -storepass android -keypass android -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 -dname 'CN=Android Debug'
```

Keep using the same key to update an existing installation without uninstalling.
Do not commit signing keys.

```bash
bash tools/package_android.sh /path/to/Turnip-v27.0.0-20261111-r2.zip
```

Output: `out/android-app/polybius.apk`. APK signing is checked by the script.
The APK includes SDL, runtime, driver and launcher resources, without game files.

## Prepare game and install

Prepare the game on the host first; see [the dump layout](../docs/building.md#supply-your-game):

```bash
python3 run.py prepare --game game
adb devices
bash tools/install_android.sh ADB_SERIAL out/game/boot-linked.bin game
adb -s ADB_SERIAL shell am start -n com.crownparkcomputing.polybius/.PolybiusActivity
```

The installer uses `install -r`, stages the image and `content.bin`, copies game
files separately.
For a runtime-only update, with game data already staged:

```bash
adb -s ADB_SERIAL install -r out/android-app/polybius.apk
```

External files are under
`/sdcard/Android/data/com.crownparkcomputing.polybius/files/`.
`polybius.log` is the main log; `launch.ini` accepts literal `TEMPEST_NAME=value`
lines. Normal play has no diagnostic timeout.

## Driver selection

Long-press the launcher icon and choose **Graphics driver**. Bundled Turnip is
the default; system-driver selection and importing another ARM64 driver ZIP are
available. Changes apply after closing and reopening the game. See
[driver settings and recovery](../docs/android-driver-selection.md).

## Defaults and checks

The initial profile uses the game's 1080p mode with 1280×720 host output,
FEX MaxInst500, serial draw recording and draw preparation on the GPU thread.
Host images use one sample, and shaders compile on demand.
Explicit `launch.ini` values override defaults. Normal play has no timeout.

```bash
bash tools/run_android_fex.sh ADB_SERIAL
```

See [port status](../docs/polybius-port-status.md) for the actual device checks
completed so far. These are separate from the shared runtime's earlier games.
Both platforms use CUSA08328 save identity, in independent local profiles.
Device rendering/performance checks are in progress; this guide is a build
recipe, not a completed compatibility claim.

L3 + R3 opens Settings. Choose Graphics driver to change the bundled/system
selection or import another compatible ARM64 driver ZIP, then relaunch.
