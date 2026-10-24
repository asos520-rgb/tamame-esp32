# Contributing to TamaMe

[English](CONTRIBUTING.md) · 简体中文（待补 / pending） · [繁體中文](CONTRIBUTING.zh-TW.md)

Use synthetic data for reproducible bug reports, documentation corrections and focused changes. Describe the concrete problem, expected behavior and target board first. Do not use private settings or personal assets as fixtures.

1. Assemble the pinned upstream revision and reviewed overlay. Do not copy a private checkout or Git history into the package.
2. Make one focused change on a new branch. When adding or modifying an overlay file, update its SHA-256 in source-plan.json; this does not replace human review.
3. Run relevant native tests for functional changes and build the target board for firmware changes. State the hardware, version and operations actually tested, and explicitly mark untested areas.
4. Explain the problem, final change, validation and limits. For asset changes, document origin, modifications, redistribution rights and attribution; remove PNG metadata and private reference inputs.

Test entry point: `python3 -B -m unittest discover -s tests -p 'test_*.py'`. Avatar tests need Pillow and a native C/C++ compiler. TTS tests require CJSON_SOURCE_DIR pointing to a directory containing cJSON.c. Inspect skipped counts; skipped tests are not passes. Tests use mocks and require no real credentials.

Never commit sdkconfig, user-assets, raw recordings, NVS, bin/ELF/MAP files, complete logs, pairing credentials or private characters/voices. Choose a public commit identity and a non-private email address yourself; commit history is public content too.

Do not flash, erase or pair another person's connected device. Automated tests must not call paid AI/TTS services by default.
