# Polybius native port

Linux x86-64 and Android ARM64 port of the PS4 Polybius CUSA08328 v1.03,
based on the shared Tempest, PAC-MAN CE2 and Akka Arrh runtime.

Linux reaches playable level 1, renders the ship, track and HUD, and returns
from game over to the title. Steady sections run at 60 FPS, with shader-compilation
pauses. Non-silent audio output is verified; Android device testing is pending.
Original packages, game/ and prepared/build outputs in out/ remain local and
excluded from Git. The APK contains no original executable, level or audio data.

Android uses the separate com.crownparkcomputing.polybius package and Polybius
launcher label. The shared L3+R3 Settings menu includes Graphics driver, with
bundled Turnip by default, system selection and importing compatible ARM64 ZIPs.
Driver changes apply on the next launch. Save profiles remain separate by title.

Build Linux with `bash build.sh`, then `./play-polybius.sh --game game` after
preparing the supported dump. Shared configuration names remain TEMPEST_*.

See [build instructions](docs/building.md), [Android](android/README.md) and
[current verification status](docs/polybius-port-status.md).
