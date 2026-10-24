# Preserving settings during tests and rollback

[English](storage-safety.en.md) · [简体中文](storage-safety.zh-CN.md) · [繁體中文](storage-safety.md)

When NVS is full, incompatible or fails initialization, the public preview returns an error instead of automatically erasing NVS. This preserves data; it does not repair it. Current callers may stop or reboot on an initialization error, and no on-screen recovery operation is available yet. Stop testing and recover from your private backup; do not clear pairing as a workaround.

| Data | Storage and policy |
|---|---|
| Public pet coins, hunger, fortune and interaction progress | Independent NVS namespace `pet_pub_stats`; no migration or modification of private-version `muse_pet_v1` |
| Public scenes, clothing and accessories | Independent namespace `pet_public_v2`; no migration of the private wardrobe |
| Volume, brightness, sleep and other device settings | Shared `muse` settings; user changes can affect the private version |
| Wi-Fi, pairing and connection data | Existing SDK settings; this change does not duplicate network or identity data into a public-only area |

The public version starts with its own initial pet progress. Restoring only the original app preserves the original pet progress but does not undo shared settings changed during testing. Restoring settings to their pre-test state requires a complete private Flash snapshot from the same device.

Before writing, back up the entire current Flash, compare two reads and SHA-256 hashes, and verify the partition table and active app slot. A Flash backup can contain Wi-Fi passwords, pairing credentials, embedded keys and private assets. Keep it private; never upload it to an Issue or Release.

During testing, do not unlink cloud pairing, revoke credentials, change accounts, clear settings or enable security features that write eFuses. Flash restoration cannot restore RAM, remote state or eFuses. This preview includes no private backups or flashing scripts. On one device, a full private snapshot was restored and verified by readback before boot, and the user confirmed character, appearance and voice. That does not guarantee recovery on other devices or recovery of cloud state.
