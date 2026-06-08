# Local Install And Host Troubleshooting

Use this note for local Ableton validation builds. It is not a distribution or notarization guide.

## Build And Check

From the repo root:

```bash
/opt/homebrew/bin/cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DSYNTHIA_ENABLE_TESTS=ON
/opt/homebrew/bin/cmake --build build-release --config Release
/opt/homebrew/bin/ctest --test-dir build-release --output-on-failure
./build-release/SynthiaRender --suite core --output-dir build-release/reports/core
scripts/check-plugin-bundles.sh build-release
```

`scripts/check-plugin-bundles.sh` verifies Standalone, AU, and VST3 bundle existence, executable architecture, key Info.plist metadata, and bundled factory presets.

## Install

Install the local AU and VST3 into the current user's plugin folders:

```bash
scripts/install-local-plugins.sh build-release
```

Installed paths:

- AU: `~/Library/Audio/Plug-Ins/Components/Synthia.component`
- VST3: `~/Library/Audio/Plug-Ins/VST3/Synthia.vst3`

The install script ad-hoc signs the copied bundles for local host scanning when `codesign` is available. To skip that during debugging:

```bash
SYNTHIA_SKIP_ADHOC_SIGN=1 scripts/install-local-plugins.sh build-release
```

Run AU validation after install:

```bash
auval -v aumu SynA PkRx
```

## Uninstall

Preview removal:

```bash
scripts/uninstall-local-plugins.sh --dry-run
```

Remove only Synthia's per-user AU and VST3 bundles:

```bash
scripts/uninstall-local-plugins.sh
```

This script does not remove Ableton caches, user presets, or unrelated plugins.

## Ableton Rescan

After install or uninstall:

1. Open Ableton preferences.
2. Go to Plug-Ins.
3. Enable Audio Units and VST3.
4. Rescan plug-ins.
5. Confirm `Synthia` appears under both AU and VST3 when installed.

If Synthia does not appear after a normal rescan, quit Ableton and inspect Ableton's plugin scan logs before deleting any cache files.

Common user cache locations to inspect or move aside for a targeted rescan:

- `~/Library/Preferences/Ableton/Live 11.0.12/`
- `~/Library/Caches/Ableton/`

Do not delete broad `~/Library` folders or unrelated plugin folders. Move suspected Ableton cache files to a temporary folder first so they can be restored if the issue is unrelated.

## Local Signing Versus Distribution Signing

Ad-hoc signing is enough for local development validation on this machine. It does not produce distributable, notarized artifacts.

Distribution builds still need:

- a Developer ID Application certificate,
- hardened runtime options chosen for the final bundle shape,
- notarization submission,
- stapling where applicable,
- final verification on a clean macOS account or machine.

Those steps belong to release hardening after Ableton validation passes.
