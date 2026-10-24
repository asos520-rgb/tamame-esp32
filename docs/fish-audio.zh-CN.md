# Fish Audio 接入方案

[English](fish-audio.en.md) · [简体中文](fish-audio.zh-CN.md) · [繁體中文](fish-audio.md)

TamaMe 建议以 Fish Audio 作为选配 TTS。**作者的私人版本已亲测有效，还原私人版本后也确认声音正常。**这项经验不等于公开默认固件已启用或完成真实 TTS 验收；公开版不附作者密钥、声音 ID 或参考录音，默认只有字幕。

本页于 2026-10-24 核对 [官方 TTS API](https://docs.fish.audio/api-reference/endpoint/openapi-v1/text-to-speech) 与 [模型概览](https://docs.fish.audio/overview/capabilities)。以下是 API 使用方式与目前程序相容的建议，不保证各账号的额度、模型权限或延迟。

## 选哪个方案

| 方案 | 现况 | 密钥位置 | 适合用途 |
|---|---|---|---|
| ESP32 直连 Fish Audio | 既有固件已实现；私人版实测有效，公开版默认关闭 | 用户私人构建设置及编译后 app | 首次自用接入，沿用现有播放流程 |
| 自架后端代理 | 架构提案，尚未实现与验证 | 后端秘密设置；设备另用可撤销的访问凭证 | 后续提供共用固件、管理多台设备 |

首版先提供直连的自行构建方法。不要把作者的私人 TTS build 或一把共用密钥发给网友。

## 方案 A：自行构建，ESP32 直连

资料路径：Muse 回复文字 → 设备整理朗读文字 → HTTPS TTS → MP3 流式传输 → 设备解码与扬声器。这会将回复文字送到 Fish Audio；麦克风辨识仍走既有 Muse 流程，这个方案没有把语音辨识改接 Fish。

1. 自行取得 Fish Audio API key，选择自己有权使用的 voice model，取得其 ID。不使用作者声音作为默认，不将参考录音放进包。
2. 在包根目录建立私人 `user-assets/`，从 `examples/user-config.json` 复制设置并用本机编辑器填入。输入文件也请限制为自己可读写。
3. 只在私人文件填入 `tts_enabled=true`、自己的 `api_key` 与 `voice_id`。`model` 明确指定，例如目前范例的 `s2.1-pro-free`；依官方文件与自己账号可用范围选择，不假设方案永远免费。
4. 产生私人设置，之后依构建顺序最后加载它。

```sh
mkdir -p user-assets
cp examples/user-config.json user-assets/fish-config.json
chmod 600 user-assets/fish-config.json
# 在本机编辑上述文件，勿把真实内容贴到聊天或 Issue。
python3 tools/prepare_user_config.py \
  --input user-assets/fish-config.json \
  --output user-assets/sdkconfig.local
```

私有设置范例中的值都是占位符：

```json
{
  "tts_enabled": true,
  "api_key": "YOUR_OWN_API_KEY",
  "voice_id": "YOUR_OWN_VOICE_ID",
  "model": "s2.1-pro-free"
}
```

在已组装的 SDK `esp32/` 目录，用**新的**构建目录；最后一项改成上述私人设置的绝对路径：

```sh
idf.py -B build-fish-private -DIDF_TARGET=esp32s3 \
  -DSDKCONFIG=build-fish-private/sdkconfig \
  '-DSDKCONFIG_DEFAULTS=sdkconfig.defaults;devices/sdkconfig.muse;devices/sdkconfig.muse-waveshare-s3-175c;devices/sdkconfig.pet-public;/absolute/path/to/user-assets/sdkconfig.local' \
  -DPROJECT_VER=0.1.0-private-tts build
```

工具本身不构建、不刷机、不连网。任何加载密钥的编译结果，包括 bin、ELF、MAP、header、sdkconfig 与完整 Flash 备份，全部视为私人资料。刷写前先按 [设置保留](storage-safety.zh-CN.md) 完成自己的备份与分区核对，不直接执行整组 flash 指令。

### API 对照

目前设备程序以 `POST https://api.fish.audio/v1/tts`，使用 Bearer 认证与 `model` header；JSON 带回复 `text`、对应 `reference_id`、MP3 格式、32 kHz、64 kbps。这些音频设置符合目前官方 API，亦与现有设备解码流程相接。`language=zh-TW` 是现用请求值；官方说明 language 用于文字正规化，不保证改变口音或音色。

`voice_id` 是本机设置名称，发送时映射成 API 的 `reference_id`，不是把 API key 当 voice ID。声音 ID 虽不是 API 认证密钥，也可能连结个人的声音模型，请私人保存。

模型／账号、用量与费用请在服务端自行确认。API 回报认证、额度或服务错误时，保留字幕并显示不含原始错误内容的提示；不要无限自动重试，也不要将服务回复 body 直接公开。这是建议的错误呈现方式，完整设备提示仍待实现与验收。

## 逐步验收

先以自己的账号在服务端确认声音，再用短句进行私人设备测试，避免一开始朗读完整私人对话。

- 确认字幕、正确声音与扬声器音量，测试中文及中英混合。
- 测试打断朗读、下一回合、断网恢复，以及服务失败时是否仍能看字幕。
- 确认重新编译的公开版默认仍是空白凭证、TTS 关闭；私有 build 不加入来源 ZIP 或 Release。

这份文件没有发出真实合成请求，也没有启用作者凭证。

## 方案 B：后端代理（后续提案）

资料路径：ESP32 → 自己的 HTTPS `/tts` → Fish Audio → MP3 流式传输回设备。后端保存 Fish key；每位用户的声音设置由后端以自己的识别资料映射，避免把长效供应商 key 放进共用 app。

需新增设备端 TTS provider／代理 URL 与认证实现；**现有设置工具不支持填代理 URL，不能只改 API key 就切换。**

代理需具备：用户／设备认证、可撤销凭证、文字长度与用量限制、逾时与取消、限制音频大小、供应商错误转译，以及不记录原文／密钥的日志。设备凭证仍需保护；不要部署无认证、替所有人付费的公开转送端点。代理也会接触朗读文字，不能宣称资料完全留在设备。

## 建议对外介绍

> TamaMe 支持选配 Fish Audio 语音合成，作者私人版本已亲测有效。公开版默认提供字幕与本机互动音效；要让宠物朗读回复，请使用自己的 Fish Audio 账号及有权使用的声音，依接入文件自行设置与构建。项目不提供作者声音、共用密钥或代付额度。
