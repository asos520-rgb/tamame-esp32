# User settings and public defaults

[English](user-settings.en.md) · [简体中文](user-settings.zh-CN.md) · [繁體中文](user-settings.md)

Public defaults include no account, voice ID or Wi-Fi credentials, and enable neither external TTS nor the phone camera. Muse conversations still require pairing your own service account; they do not run offline.

## Local configuration example

From the public candidate directory, copy examples/user-config.json to user-assets/my-config.json. The default is tts_enabled=false with an empty key and voice ID.

```sh
mkdir -p user-assets
cp examples/user-config.json user-assets/my-config.json
python3 tools/prepare_user_config.py --input user-assets/my-config.json --output user-assets/sdkconfig.local
```

The tool only writes a local configuration file with mode 0600. It does not delete its input, change the SDK, build, flash, access the network or overwrite existing output. Keep the input private too.

For optional Fish Audio, set tts_enabled to true and supply your own api_key and voice_id. Choose model according to what your service account supports. The example model identifier reflects the code's configuration and does not guarantee account availability. Use only voices you are entitled to use; the author's voice and voice-cloning recordings are not supplied.

The tool rejects leftover credentials when TTS is disabled, and rejects missing keys or voice IDs when enabled. Do not paste real configuration into Issues, social posts or AI chats.

## Before building

A public-default build with TTS disabled has passed. Real TTS service operation has not been tested in that public build. Load settings in this order: SDK defaults → Muse defaults → 1.75C hardware defaults → sdkconfig.pet-public → your optional private settings. Use a new build directory rather than a private build's sdkconfig.

TTS keys and voice IDs are compiled into the application. Treat such a build as private: never distribute its bin, ELF, MAP, generated headers or sdkconfig. Publicly downloadable firmware must be rebuilt from clean settings without credentials.

Configure your own Muse SDK Token through the actual device/pairing flow. This tool does not accept, store or copy that Token. Wi-Fi and pairing data are not taken from the author's backups.

## Missing settings and errors

- TTS disabled: this module starts neither external TTS nor attachment-audio download workers; the caption flow remains available.
- TTS enabled but key/voice ID incomplete: text TTS does not start. Attachment-audio GET is a separate path and may still be triggered by a reply. An absent TTS key does not imply that all network traffic is disabled.
- Service errors: mocks verified worker failure and cleanup. Public-build end-to-end TTS error presentation and listening quality remain unverified.

A working caption code path or host mock is not proof of live TTS playback. Local interaction sound on one device has been confirmed separately; see [validation status](validation-status.en.md).

## Recommended Fish Audio integration

The author personally verified Fish Audio in their private build. Public defaults contain no credentials and do not speak replies. See [Fish Audio integration](fish-audio.en.md) for setup, API mapping, private builds and the proposed backend proxy. Private-build success is not public-default TTS acceptance.

## Priority font milestone

Public defaults bundle the Traditional/Simplified union font: Big5 primary, GB2312 level 1, current UI characters and retained symbols, totaling 7,379 codepoints. No post-install font download is required. Captions and Muse replies are not translated. The English / 简体中文 / 繁體中文 selector and central UI catalog are implemented. English is the default; saving a preference to public NVS restarts the device. Rare characters, emoji and other scripts are not guaranteed. See [font scope and tests](font-coverage.md).

## Interface language

Open Settings and scroll to Language. Choose English, 简体中文 or 繁體中文. The selection is saved under public NVS namespace `tamame_ui`, key `language`, then the device restarts to rebuild every page consistently. Save failure keeps the current interface and does not restart. Fresh devices, missing preferences and invalid values default to English; reads never erase storage. The selection changes fixed interface text and daily fortunes, not Muse replies, transcripts, Wi-Fi names or TTS voices. Restart clears the volatile recent-message list; pairing, network and wardrobe settings are preserved. A complete transfer/erase workflow remains pending; language selection is not a factory reset.

## Wardrobe accessories

Wardrobe has three tabs: Scene, Clothes and Accessories. Choose one accessory (or No accessory); Sunglasses is an independent checkbox and can be worn with that accessory. Choosing No accessory leaves the sunglasses setting unchanged. Tap Sunglasses again to remove only the glasses. Choices preview immediately; Apply saves both settings, and Cancel restores the saved outfit. Existing accessory IDs and the sunglasses flag are preserved, so no wardrobe migration or reset is required. Scene choices scroll vertically.
