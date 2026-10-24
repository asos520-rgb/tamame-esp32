# 用户设置与公开版默认

[English](user-settings.en.md) · [简体中文](user-settings.zh-CN.md) · [繁體中文](user-settings.md)

公开版默认不带账号、不带声音 ID、不带 Wi-Fi、不开启外部 TTS 或手机摄影机。这不代表 Muse 对话可以离线运作；AI 对话仍需以自己的账号配对服务。

## 本机设置范例

在公开候选目录中执行，先把 examples/user-config.json 复制到 user-assets/my-config.json。默认 tts_enabled=false，密钥与声音 ID 都留空。

```sh
mkdir -p user-assets
cp examples/user-config.json user-assets/my-config.json
python3 tools/prepare_user_config.py --input user-assets/my-config.json --output user-assets/sdkconfig.local
```

工具只在本机产生权限 0600 的配置文件，不删除输入、不修改 SDK、不构建、不刷机、不连网。既有输出不覆盖。请把输入也留在私人目录。

若选用 Fish Audio，将 tts_enabled 改成 true，再填入自己的 api_key、voice_id，model 则依服务账号实际支持值设置；范例模型名称来自目前程序设置，不保证账号可用性。声音需有权使用，不提供作者声音或任何声音复制素材。

停用时工具拒绝残留密钥；启用时缺少密钥或声音 ID 也会拒绝产生设置。不要把真实设置贴进 Issue、社群或 AI 对话中。

## 构建前必读

第四阶段已完成默认关闭 TTS 的公开设置构建，尚未验证真实服务。加载设置的顺序为：SDK 默认 → Muse 默认 → 1.75C 硬件默认 → sdkconfig.pet-public → 自己的私人设置（选配）。必须使用新构建目录，不能沿用私人版 sdkconfig。

选用 TTS 的密钥和声音 ID 会编译进应用程序，这种构建结果必须视为私人文件；不要分享它的 bin、ELF、MAP、产生的 header 或 sdkconfig。公开可下载固件只能从干净、未带凭证的设置重新构建。

Muse SDK Token 保留由用户自行在设备／配对流程设置；本工具不输入、不保存、不复制此 Token。Wi-Fi 与配对资料不从作者设备备份取得。

## 缺少设置时

- TTS 停用：不启动本模块的外部 TTS 或附件音频下载 worker；已有字幕流程保留。
- TTS 已开但密钥／声音 ID 不完整：文字 TTS 不启动。附件音频的 GET 是独立流程，仍可能由回复触发，因此不应以「没填 TTS 密钥」推定没有任何网络请求。
- 服务错误：本机模拟验证了 worker 失败与回收；公开版完整 TTS 的错误提示与听感仍待验证。

不能把可见字幕的程序路径或主机 mock 测试，当作真实 TTS 播放已通过验证。本机互动音效已由单台实机用户另行确认，详见 [验证状态](validation-status.zh-CN.md)。

## Fish Audio 建议接入

作者私人版本已亲测 Fish Audio 有效；公开版默认不带凭证、不开启朗读。完整操作、API 对照、私人构建与后端代理提案见 [Fish Audio 接入方案](fish-audio.zh-CN.md)。私人版本的成功不等于公开版默认 TTS 已验收。

## 优先字体支持进度

公开版默认将繁简联合子集字体编译进固件，覆盖 Big5 一级、GB2312 一级、当前 UI 用字及既有符号，共 7,379 个码位，无需安装后补装字体。此阶段不翻译字幕或 Muse 回复。English／简体中文／繁體中文设置切换和统一三语字符串表已实现；默认英文，保存至公开版 NVS 后重启生效。生僻字、emoji 及其他文字系统不在保证范围。见 [字体覆盖与测试](font-coverage.md)（英文）。

## 界面语言

打开设置并滚动至语言区域，选择 English、简体中文或繁體中文。选择会保存到公开版 NVS 命名空间 `tamame_ui` 的 `language`，然后重启以统一重建所有页面。保存失败时保留当前界面且不重启。新设备、缺少设置或无效值默认英文；读取不会清除存储数据。此设置只更改固定界面和每日运势，不翻译 Muse 回复、识别字幕、Wi-Fi 名称，也不更改 TTS 声音。重启会清除暂存的最近消息，配对、网络和换装设置保留。完整转让／清除流程仍待验收；切换语言不是恢复出厂设置。

## 换装饰品

换装分为背景、服装、饰品三个分类。配件选一或选“不加配件”；墨镜可独立勾选，与配件同时佩戴。“不加配件”只清除配件，保留墨镜状态；再点墨镜即可单独取下。选择立即预览，应用保存两种设置，取消恢复已保存穿搭。既有配件编号和墨镜标记保留，无需迁移或重置穿搭。背景选项可上下滚动。
