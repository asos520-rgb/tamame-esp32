# Local source assembly and build preview

[English](build-preview.en.md) · [简体中文](build-preview.zh-CN.md) · [繁體中文](build-preview.md)

The only target is Waveshare ESP32-S3-Touch-AMOLED-1.75C with ESP-IDF 6.0.1. An isolated build passed, followed by initial checks on one device: display, a background-switching retest, reboot persistence, captions and local sound effects. Other flows remain unverified; see [validation status](validation-status.en.md).

## Assemble the source

Obtain Muse Gadget SDK separately. The tool reads Git objects from the commit pinned in source-plan.json. It does not read private checkout modifications, untracked files or sdkconfig, and does not copy Git history.

Run from this preview package's root, replacing the upstream path with your own SDK repository:

```sh
mkdir -p user-assets
python3 tools/assemble_sdk.py --upstream /path/to/muse-gadget-sdk --output user-assets/public-sdk
```

The tool checks the manifest and four required artwork files. It never overwrites an existing output directory, downloads dependencies, builds or flashes. Install and activate ESP-IDF yourself. Initial managed-component resolution may require internet access; a third-party cache is not an author's private configuration, but its lock and licences still need review.

## Clean build

Activate your ESP-IDF 6.0.1 environment and run in the assembled public-sdk/esp32 directory:

```sh
idf.py -B build-public -DIDF_TARGET=esp32s3 \
  -DSDKCONFIG=build-public/sdkconfig \
  '-DSDKCONFIG_DEFAULTS=sdkconfig.defaults;devices/sdkconfig.muse;devices/sdkconfig.muse-waveshare-s3-175c;devices/sdkconfig.pet-public' \
  -DPROJECT_VER=0.1.0-public-preview build
```

Do not reuse a private build directory. The public defaults contain no SDK Token, Wi-Fi credentials, pairing data or TTS credentials; a blank SDK Token produces an expected build warning. Fresh-account pairing and service availability still need verification. Configure your own Token according to the actual service flow.

To reduce local paths embedded in build artifacts, optionally pass C/C++ -ffile-prefix-map flags mapping your source root to generic build paths. Configure this locally; do not commit your actual home directory in public instructions.

The validated application is 3,280,896 bytes. Its 4,194,304-byte app partition has 913,408 bytes free (about 21.78%). Exact results are retained in local validation reports. No downloadable firmware bin, ELF or NVS backup has been released.

## Verification boundaries

Validated: production asset formats, actual C-renderer state matrices, native memory checks, configuration and TTS mocks, and the ESP-IDF build. The image at ../docs/images/public-render-review-v2.png is a renderer preview, not a hardware screenshot.

Still pending: the complete touch/interaction matrix, fresh pairing, public-build TTS, missing-account and missing-voice-setting prompts, a full log audit, long-running stability and device-clearing flows. Do not run the upstream full-device flash/erase commands without a separate device-specific plan. The preview assembly tool does not operate hardware.
