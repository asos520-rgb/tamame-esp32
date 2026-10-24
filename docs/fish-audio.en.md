# Fish Audio integration

[English](fish-audio.en.md) · [简体中文](fish-audio.zh-CN.md) · [繁體中文](fish-audio.md)

TamaMe recommends Fish Audio as an optional TTS provider. **The author personally verified it in their private build and confirmed voice playback after restoring that build.** This is not evidence that the public-default firmware enables TTS or has passed a live TTS test. Public defaults include no author key, voice ID or reference recording and display captions only.

The [official TTS API](https://docs.fish.audio/api-reference/endpoint/openapi-v1/text-to-speech) and [model overview](https://docs.fish.audio/overview/capabilities) were checked on 2026-10-24. These instructions describe the API and compatibility with current code; they do not guarantee account quotas, model access or latency.

## Choose an approach

| Approach | Status | Key location | Use case |
|---|---|---|---|
| ESP32 directly calls Fish Audio | Implemented in existing firmware; private build tested, public default disabled | User's private build settings and compiled app | Initial personal setup using the existing playback flow |
| Self-hosted backend proxy | Architecture proposal, not implemented or validated | Backend secret settings; device uses a separate revocable credential | Future shared firmware and multiple devices |

The first version documents direct connection with a private build. Do not distribute the author's private TTS build or a shared key to other users.

## Approach A: private build, direct ESP32 connection

Data path: Muse reply text → device prepares speech text → HTTPS TTS → MP3 stream → device decoding and speaker. Reply text is sent to Fish Audio. Microphone recognition remains in the existing Muse flow; this approach does not move recognition to Fish.

1. Obtain your own Fish Audio API key and the ID of a voice model you are entitled to use. Do not use the author's voice as a default or include reference recordings in the package.
2. Create private `user-assets/` at the package root. Copy `examples/user-config.json` and edit it locally. Restrict input-file access to yourself too.
3. Only in that private file, set `tts_enabled=true`, your `api_key` and your `voice_id`. Specify `model` explicitly, for example the current example's `s2.1-pro-free`. Follow official documentation and your account's availability; do not assume the tier will always be free.
4. Generate private settings and load them last in the build-defaults sequence.

```sh
mkdir -p user-assets
cp examples/user-config.json user-assets/fish-config.json
chmod 600 user-assets/fish-config.json
# Edit locally. Never paste real values into a chat or Issue.
python3 tools/prepare_user_config.py \
  --input user-assets/fish-config.json \
  --output user-assets/sdkconfig.local
```

All credential values below are placeholders:

```json
{
  "tts_enabled": true,
  "api_key": "YOUR_OWN_API_KEY",
  "voice_id": "YOUR_OWN_VOICE_ID",
  "model": "s2.1-pro-free"
}
```

In the assembled SDK's `esp32/` directory, use a **new** build directory. Replace the last defaults entry with the absolute path to your private settings:

```sh
idf.py -B build-fish-private -DIDF_TARGET=esp32s3 \
  -DSDKCONFIG=build-fish-private/sdkconfig \
  '-DSDKCONFIG_DEFAULTS=sdkconfig.defaults;devices/sdkconfig.muse;devices/sdkconfig.muse-waveshare-s3-175c;devices/sdkconfig.pet-public;/absolute/path/to/user-assets/sdkconfig.local' \
  -DPROJECT_VER=0.1.0-private-tts build
```

The configuration tool does not build, flash or connect to the network. Every credential-bearing build artifact—including bin, ELF, MAP, headers, sdkconfig and full Flash backups—is private. Before flashing, follow [storage safety](storage-safety.en.md) to make your own backup and verify partitions; do not directly run the full-device flash command.

### API mapping

Current device code sends `POST https://api.fish.audio/v1/tts` with Bearer authentication and a `model` header. Its JSON contains reply `text`, `reference_id`, MP3 format, 32 kHz and 64 kbps. These audio settings match the checked API documentation and existing device decoding flow. The code uses `language=zh-TW`; the API describes language as a text-normalization setting, not a guarantee of accent or voice changes.

Local `voice_id` maps to API `reference_id`; an API key is not a voice ID. A voice ID is not an API authentication key, but may identify a person's voice model, so keep it private.

Check your model access, account usage and charges with the provider. For authentication, quota or service errors, preserve captions and use a prompt without raw error details; do not retry indefinitely or publish the response body. This describes the intended error-handling presentation; complete device prompts remain a release task.

## Acceptance checks

First verify the voice with your own provider account, then test a short sentence on your private device rather than a whole private conversation.

- Check captions, correct voice and speaker volume; test Chinese and mixed Chinese/English text.
- Test interruption, the next turn, reconnecting after network loss, and captions when the provider fails.
- Confirm public-default rebuilds still have blank credentials and TTS disabled. Never put a private build into the source ZIP or Release.

Writing this guide did not issue a live synthesis request or enable author credentials.

## Approach B: backend proxy — future proposal

Data path: ESP32 → your HTTPS `/tts` endpoint → Fish Audio → MP3 stream back to the device. The backend stores the Fish key and maps each user's voice settings through its own identity model, keeping a long-lived provider key out of shared firmware.

This requires a new device-side TTS provider/proxy URL and authentication implementation. **The current configuration tool accepts no proxy URL; replacing an API key alone cannot switch to this architecture.**

The proxy needs user/device authentication, revocable credentials, text-length and usage limits, timeouts and cancellation, bounded audio sizes, provider-error translation, and logs without raw text or keys. Device credentials still need protection. Do not deploy an unauthenticated endpoint that pays for everyone's synthesis. The proxy sees speech text too; this is not wholly on-device processing.

## Suggested public description

> TamaMe supports optional Fish Audio speech synthesis, personally verified by the author in a private build. Public defaults provide captions and local interaction sounds. To speak replies, use your own Fish Audio account and an authorized voice, then follow the integration guide to configure and build. The project supplies no author voice, shared key or sponsored quota.
