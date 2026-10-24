# Documentation languages

Priority: **English → 简体中文 → 繁體中文**. English is the default repository landing page. The firmware now defaults to English and has a persisted English / 简体中文 / 繁體中文 selector; saving restarts the device. Targeted language-switch and wardrobe hardware checks passed; the full validation matrix remains incomplete.

| Document | English | 简体中文 | 繁體中文 |
|---|---|---|---|
| Project overview | [README.md](README.md) | [README.zh-CN.md](README.zh-CN.md) | [README.zh-TW.md](README.zh-TW.md) |
| Assembly and build | [Guide](docs/build-preview.en.md) | [指南](docs/build-preview.zh-CN.md) | [指南](docs/build-preview.md) |
| User settings | [Guide](docs/user-settings.en.md) | [指南](docs/user-settings.zh-CN.md) | [指南](docs/user-settings.md) |
| Fish Audio integration | [Guide](docs/fish-audio.en.md) | [指南](docs/fish-audio.zh-CN.md) | [指南](docs/fish-audio.md) |
| Validation status | [Guide](docs/validation-status.en.md) | [指南](docs/validation-status.zh-CN.md) | [指南](docs/validation-status.md) |
| Storage and rollback safety | [Guide](docs/storage-safety.en.md) | [指南](docs/storage-safety.zh-CN.md) | [指南](docs/storage-safety.md) |
| Privacy | [Guide](docs/privacy.en.md) | [指南](docs/privacy.zh-CN.md) | [指南](docs/privacy.md) |
| Contributing | [CONTRIBUTING.md](CONTRIBUTING.md) | Pending | [CONTRIBUTING.zh-TW.md](CONTRIBUTING.zh-TW.md) |
| Support | [SUPPORT.md](SUPPORT.md) | Pending | [SUPPORT.zh-TW.md](SUPPORT.zh-TW.md) |
| Security | [SECURITY.md](SECURITY.md) | Pending | [SECURITY.zh-TW.md](SECURITY.zh-TW.md) |

## Maintainer guides

The following remain in Traditional Chinese, each with a one-line English summary at the top. Complete English and Simplified Chinese translations are outside this documentation pass:

- [Avatar contract](docs/avatar-contract.md)
- [Avatar tools](docs/avatar-tools.md)
- [Data flow](docs/data-flow.md)
- [Public artwork specification](docs/public-art-spec.md)
- [Release checklist](docs/release-checklist.md)
- [Source layout](docs/source-layout.md)
- [Third-party review](docs/third-party-review.md)

Source licences and third-party attribution texts retain their original wording. Asset/renderer notes, issue/PR templates and the generated preview image's labels are not fully localized in this pass. A translated guide does not mean a localized device UI or a new hardware validation result.

## Translation maintenance

Keep commands, paths, JSON keys, configuration symbols, storage namespaces and validation limits consistent across translations. Preserve the distinction between author-tested private Fish Audio integration and public-default TTS being disabled. Do not add private account names, credentials, device logs or new service guarantees to a translation.

Update all three core user-guide variants together when behavior changes. Community Simplified Chinese translations remain a stated follow-up. Publication owner and public attribution remain maintainer decisions; this package does not publish or create a repository.

## Bundled font milestone

[Font coverage](docs/font-coverage.md) documents the 7,379-codepoint common English/Traditional/Simplified repertoire and actual native LVGL sample rendering. The expanded font is pending hardware acceptance. A central three-language UI catalog and English-default selector are implemented. Public NVS stores the preference; saving restarts the device. Targeted language-switch and wardrobe hardware results are recorded in the validation-status guides; exhaustive caption and error-path acceptance is still pending.
