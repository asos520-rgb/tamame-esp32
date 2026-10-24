# 本机源码组装与构建预览

[English](build-preview.en.md) · [简体中文](build-preview.zh-CN.md) · [繁體中文](build-preview.md)

目标只有 Waveshare ESP32-S3-Touch-AMOLED-1.75C，ESP-IDF 6.0.1。已在独立目录编译成功，并完成单台设备的首轮画面、背景切换重测、重启保存、字幕与本机音效确认；仍有未验证流程，详见 [验证状态](validation-status.zh-CN.md)。

## 源码组装

先自行取得 Muse Gadget SDK。工具只读 source-plan.json 固定 commit 的 Git 物件，不读 checkout 中的私密修改、未追踪文件或 sdkconfig，也不复制 Git 历史。

在此预览包根目录执行；将路径替换成自己的 SDK 储存库位置：

```sh
mkdir -p user-assets
python3 tools/assemble_sdk.py --upstream /path/to/muse-gadget-sdk --output user-assets/public-sdk
```

工具先核对来源与四份必要素材，不覆盖既有目录、不下载、不构建、不刷机。后续需自行安装／启用 ESP-IDF。首次解析管理元件可能需要连网；既有第三方快取不是作者设置，仍应核对 lock 与授权。

## 干净构建

在新组装的 public-sdk/esp32 目录执行，先启用自己的 ESP-IDF 6.0.1 环境：

```sh
idf.py -B build-public -DIDF_TARGET=esp32s3 \
  -DSDKCONFIG=build-public/sdkconfig \
  '-DSDKCONFIG_DEFAULTS=sdkconfig.defaults;devices/sdkconfig.muse;devices/sdkconfig.muse-waveshare-s3-175c;devices/sdkconfig.pet-public' \
  -DPROJECT_VER=0.1.0-public-preview build
```

不得沿用私人构建目录。公开预览不带 SDK Token、Wi-Fi、配对资料或 TTS 凭证；编译会提示 SDK Token 空白。真实配对／服务可用性仍待验证，用户需要依实际服务流程设置自己的 Token。

若要减少编译产物包含本机路径，可额外传入 C/C++ 的 -ffile-prefix-map，将自己的来源根目录映射到通用 build 路径。请在本机设置，不要把真实主目录写进公开说明或提交。

本轮产物检查：应用映像 3,280,896 bytes，4,194,304-byte app 分区剩余 913,408 bytes（约 21.78%）；实际数值记于本机验证报告。未发布可下载的固件 bin／ELF／NVS 备份。

## 验证界线

已验证正式素材格式、实际 C renderer 的状态矩阵、原生内存检查、设置与 TTS mock，以及 ESP-IDF 编译。docs/images/public-render-review-v2.png 是实际渲染图，不是实机截图。

尚待：完整触控与互动矩阵、全新配对、TTS、缺少账号与语音设置时的提示、完整日志审查、长时间稳定性及清除资料流程。不要直接执行上游列出的全区 flash／erase 命令；本预览工具不会操作设备。
