# TamaMe validation status

[English](validation-status.en.md) · [简体中文](validation-status.zh-CN.md) · [繁體中文](validation-status.md)

Updated 2026-10-24. Source preview, release_ready=false; not publicly published.

| Area | Evidence and limits |
|---|---|
| Build | Isolated ESP-IDF 6.0.1 / ESP32-S3 / 1.75C build passed; app 3,280,896 bytes, leaving 913,408 bytes in a 4 MiB slot |
| Native checks | 36 candidate tests and 8 privacy-tool tests passed; tests initially skipped for a missing cJSON path were subsequently rerun successfully |
| Display and menus | User confirmed a normal display and opening menus on one device; not all touch regions or operations were verified |
| Background | Initial manual apply failure was reported; command-based apply and two subsequent manual retests worked. The cause remains unknown; an intermittent defect has not been declared fixed |
| Wardrobe persistence | Device queries reported background 2 and clothing 2 before and after a reset; the user confirmed normal appearance |
| Conversation and captions | The user spoke and saw captions with existing pairing. This is not a fresh-pairing test or a full speech-recognition quality assessment |
| Speaker | User heard local pet/coin interaction sounds; not proof of all formats or TTS |
| Fish Audio TTS | Author's private build worked and voice returned after restoration. Public defaults disable TTS; private credentials were not loaded for a live public-build TTS test |
| Full rollback | Two 32 MiB snapshots matched. A complete write-back and full byte-for-byte readback matched before boot; pairing and service recovered, and the user confirmed private character, appearance and voice |
| Privacy | Exact-file manifests, heuristic text rules and reviewed PNG checks passed; not an audit of every upstream runtime log or remote retention policy |

Pending: new-account/device pairing, all interactions and error paths, long-running operation, complete device clearing/transfer, a missing-TTS-settings device prompt, final third-party/artwork rights review, a private security-reporting channel and publication settings.

The test device has been fully restored to its private version. The public package contains none of its identity, credentials, snapshots, raw logs or private assets. This page records sanitized results only.

## Priority font milestone

The bundled font now covers 7,379 codepoints, including Big5 primary, GB2312 level 1, current UI characters and symbols. Actual native LVGL checked the complete repertoire and rendered one synthetic reply per language with zero missing glyphs and unchanged input text. This is a host test, not hardware caption acceptance. Language switching and a central three-language UI catalog are implemented, with a restart after saving; targeted UI checks are recorded below, while exhaustive physical caption testing remains pending; rare characters and emoji are not guaranteed. See [font testing](font-coverage.md).

## Three-language interface validation

The public firmware now defaults to English and uses a single 323-entry catalog for fixed settings, menus, status/error messages, interactions, wardrobe and daily fortunes. Settings offers English / 简体中文 / 繁體中文. The preference is saved only to public NVS `tamame_ui` / `language`; saving restarts the device so all static labels update. Missing or invalid preferences use English; storage errors never erase NVS. A failed save does not restart and displays a settings-page error.

36 native candidate tests and 8 privacy-tool tests pass. Native LVGL checks all catalog translations and the 7,379-codepoint font, and renders one unmodified synthetic reply per language without missing glyphs. The clean ESP32-S3 / 1.75C firmware build and slot budget are recorded below. These results do not establish physical touch/layout acceptance.

For broader acceptance, still check physical long-caption wrapping, all scenes, food/fortune pages, remaining touch targets, and save-failure feedback. Obtain authorization and a fresh full backup before any future flashing. Targeted completed checks are listed below; recent messages are intentionally volatile. The firmware has been flashed for controlled acceptance; no public release was published.

The first physical English check reported overflow in Wardrobe. The updated three-tab layout merges sunglasses into Accessories as an independent checkbox and passed real production LVGL label-fit and interaction tests in all three languages (art preview is stubbed in the native harness). The updated app was flashed and verified; the user confirmed the corrected English wardrobe in the subsequent physical retest. This replaces the earlier layout acceptance assumption.

## Latest targeted hardware acceptance — 2026-10-24

On one original 1.75C device, the user confirmed the corrected English wardrobe layout and simultaneous accessory/sunglasses selection. They then selected Simplified Chinese and Traditional Chinese in Settings; each automatic restart retained the selected language, and the requested settings/wardrobe pages displayed and operated normally. After a user-performed shutdown/startup, Traditional Chinese and the saved background/accessory/sunglasses outfit remained, and Wi-Fi connected automatically.

This is targeted user-observed acceptance, not exhaustive testing of every English settings screen, fortune/food page, error path, all scenes or long-caption pagination. Common glyph coverage and the three reply samples remain supported by native LVGL evidence; arbitrary Unicode and rare characters are not guaranteed. No fresh-account pairing or public-default live TTS claim is added.

After testing, the current pre-test full Flash backup was restored. All 32 MiB matched byte-for-byte on readback before boot. Private firmware, Wi-Fi and paired/Online connection recovered; the user confirmed their private character, appearance and voice were normal. This does not back up or roll back remote service state.
