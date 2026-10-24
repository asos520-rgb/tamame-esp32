# Privacy and data handling — pre-publication draft

[English](privacy.en.md) · [简体中文](privacy.zh-CN.md) · [繁體中文](privacy.md)

Public materials must not include anyone's network passwords, Tokens, voice IDs, private voice recordings, conversations, pairing keys or complete device backups.

The existing recent-reply implementation holds the latest eight received replies in RAM and clears them on reboot; public-build behavior needs renewed verification. This does not imply that remote services retain no conversations. Their data handling must be documented separately for the services actually used.

Voice conversations may send microphone audio or recognized text to the configured service. TTS may send reply text to a speech provider. See the code-based [data-flow inventory](data-flow.md) (Traditional Chinese with an English summary) and [user settings](user-settings.en.md); full integration and live-service verification remain incomplete. Use only voices you are entitled to use. No real-person voice-cloning recordings are included by default.

Native and automated tests use synthetic or public artwork inputs. One hardware test used the author's existing pairing, but private conversations, credentials and raw logs are excluded from the package. Diagnostics and bug reports must not contain full conversations, passwords or credentials. Avatar PNGs, generation prompts and compiled arrays can contain identifying features; keep your personal assets private.

No complete, verified whole-device clearing flow is provided. Reflashing an app does not delete network or pairing settings. A device transfer requires separate, explicit clearing instructions and verification.
