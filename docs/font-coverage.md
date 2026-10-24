# Bundled font coverage — priority milestone

The public profile compiles a 16px, 4bpp Noto CJK union font into firmware. Users do not need to install a font after flashing. This milestone addresses the bundled glyph repertoire; the three-language UI setting and central translation catalog are now implemented, defaulting to English. Saving the language restarts the device; targeted UI language-switch and wardrobe device checks passed; exhaustive physical caption coverage remains pending.

## Repertoire and strategy

The subset includes Big5 first-level ideographs (A440–C67E: 5,401 characters), GB2312 level-1 ideographs (B0A1–D7F9: 3,755 characters), the current UI's CJK characters, the previous symbol repertoire, printable ASCII, common Latin-1 accented letters, and the language names English / 简体中文 / 繁體中文. Shared Unicode codepoints occur only once.

The two primary Chinese sets contain 6,722 distinct codepoints after deduplication; retaining symbols and the other requirements produces 7,379 glyphs. Two independent primary fonts would duplicate 2,434 primary codepoints. A single union avoids those duplicate bitmaps and duplicate fallback metadata. No claim is made about the binary size of an unbuilt dual-font alternative.

The original regional TC subset lacked many Simplified Chinese glyphs. Generation now uses the complete Noto Sans CJK TC font at a pinned upstream revision, then selects only the required codepoints. The full desktop OTF is not linked into firmware. Common characters shared by regions use the TC source's regional glyph shape; separate region-specific glyph forms would require a different font strategy.

See the compiled font's provenance, SHA-256 and exact counts in [generation metadata](../firmware-overlay/esp32/components/muse/fonts/multilingual-coverage.json) and its [OFL licence](../firmware-overlay/esp32/components/muse/fonts/NotoSansTC-OFL.txt).

This is a defined common repertoire, not all Unicode. Rare personal/place-name characters, emoji, historic ideographs and other scripts are not guaranteed. Do not promise that any arbitrary remote reply can never contain an unsupported character. Caption and Muse reply content is not translated, simplified or replaced by this font change.

## Native checks

`tests/test_multilingual_font.py` fails if the defined primary sets, current UI characters or synthetic reply samples are missing from the shipped generated font. It also verifies generation hashes.

The additional native harness compiles actual LVGL, the bundled font and the production UI fallback function. It checks all 7,379 codepoints, renders one synthetic reply in each language, rejects placeholder glyphs and asserts that the label's text equals its input. Screenshots are host LVGL renders, not physical-device evidence.

Use local sources from the pinned SDK; output must be a new private work directory:

```sh
python3 tools/check_multilingual_font_native.py \
  --lvgl /path/to/public-sdk/esp32/managed_components/lvgl__lvgl \
  --muse-includes /path/to/public-sdk/esp32/components/muse \
  --output user-assets/font-native-check
```

CMake and a native C/C++ compiler are required; optionally select CMake with --cmake. This command does not use AI/TTS services or flash hardware.

## Regeneration

The generated C source is already bundled. Regeneration additionally needs the local OTF referenced by generation metadata, Node, and lv_font_conv 1.5.3. Download tools/assets separately; the generator is offline and checks the source checksum, converter version, cmap coverage and generated glyph count before accepting output.

```sh
python3 tools/prepare_multilingual_font.py \
  --font /path/to/NotoSansCJKtc-Regular.otf \
  --converter /path/to/lv_font_conv/lv_font_conv.js \
  --node node
```

The source generator and glyph tests must run again whenever the translation catalog adds characters. Firmware must then be built in a fresh directory and its actual app size checked against the 4 MiB app slot. A font milestone alone does not complete language switching or hardware acceptance.

## Measured app budget

A fresh ESP-IDF 6.0.1 ESP32-S3 / 1.75C build passed with blank SDK Token and external TTS disabled. All 72 overlay file hashes match the built source.

| Measurement | Previous public build | Font milestone |
|---|---:|---:|
| App image | 3,084,288 bytes | 3,280,896 bytes |
| Free in 4,194,304-byte app slot | 1,110,016 bytes | 913,408 bytes (21.78%) |
| Compiled font object | 745,566 bytes | 921,431 bytes |

The app image increased by 196,608 bytes; the font object increased by 175,865 bytes. App image alignment and other build layout overhead mean those deltas need not match. The font object has zero static data/BSS bytes in the size-tool report; this is not a complete runtime heap assessment.

36 candidate tests and 8 privacy-tool tests passed without skips, in addition to actual native LVGL glyph and label rendering. The expanded-font firmware has not been flashed or accepted on hardware. The private device/version was not changed.

## Mixed Chinese and Latin spacing correction

Latin letters, digits and spaces retain the Noto source font’s natural advances; Chinese characters retain full-width advances. The earlier generated font incorrectly forced all nonzero advances to 16 pixels. Reply layout now explicitly measures a Chinese cell rather than the proportional letter M. Pagination still conservatively budgets full cells, so some mixed lines can turn early; pixel-optimal pagination is not claimed. No user reply text is converted or translated. Device acceptance remains pending; this change is not flashed.

## Three-language UI integration

The single catalog `muse_i18n_catalog.def` contains 323 complete English / Simplified / Traditional entries, including printf-format parity checks. The native LVGL harness checks every translated string against the font as well as the three unchanged reply samples. UI language is loaded before UI construction; Settings saves a validated value in the separate `tamame_ui` NVS namespace and restarts. This intentionally rebuilds static labels instead of leaving mixed-language cached pages. Saving failure is shown in Settings and does not restart. The catalog does not translate replies, transcripts, SSIDs or other user content. Font regeneration includes `.def` catalog characters; the 7,379-codepoint union already covers the complete catalog. Language selection and targeted wardrobe hardware checks passed. Exhaustive physical caption/font acceptance remains pending.
