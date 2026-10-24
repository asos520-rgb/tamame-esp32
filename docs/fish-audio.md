# Fish Audio 串接方案

[English](fish-audio.en.md) · [简体中文](fish-audio.zh-CN.md) · [繁體中文](fish-audio.md)

TamaMe 建議以 Fish Audio 作為選配 TTS。**作者的私人版本已親測有效，還原私人版本後也確認聲音正常。**這項經驗不等於公開預設韌體已啟用或完成真實 TTS 驗收；公版不附作者金鑰、聲音 ID 或參考錄音，預設只有字幕。

本頁於 2026-10-24 核對 [官方 TTS API](https://docs.fish.audio/api-reference/endpoint/openapi-v1/text-to-speech) 與 [模型概覽](https://docs.fish.audio/overview/capabilities)。以下是 API 使用方式與目前程式相容的建議，不保證各帳號的額度、模型權限或延遲。

## 選哪個方案

| 方案 | 現況 | 金鑰位置 | 適合用途 |
|---|---|---|---|
| ESP32 直連 Fish Audio | 既有韌體已實作；私人版實測有效，公版預設關閉 | 使用者私人建置設定及編譯後 app | 首次自用串接，沿用現有播放流程 |
| 自架後端代理 | 架構提案，尚未實作與驗證 | 後端秘密設定；設備另用可撤銷的存取憑證 | 後續提供共用韌體、管理多台設備 |

首版先提供直連的自行建置方法。不要把作者的私人 TTS build 或一把共用金鑰發給網友。

## 方案 A：自行建置，ESP32 直連

資料路徑：Muse 回覆文字 → 裝置整理朗讀文字 → HTTPS TTS → MP3 串流 → 裝置解碼與喇叭。這會將回覆文字送到 Fish Audio；麥克風辨識仍走既有 Muse 流程，這個方案沒有把語音辨識改接 Fish。

1. 自行取得 Fish Audio API key，選擇自己有權使用的 voice model，取得其 ID。不使用作者聲音作為預設，不將參考錄音放進套件。
2. 在套件根目錄建立私人 `user-assets/`，從 `examples/user-config.json` 複製設定並用本機編輯器填入。輸入檔也請限制為自己可讀寫。
3. 只在私人檔案填入 `tts_enabled=true`、自己的 `api_key` 與 `voice_id`。`model` 明確指定，例如目前範例的 `s2.1-pro-free`；依官方文件與自己帳號可用範圍選擇，不假設方案永遠免費。
4. 產生私人設定，之後依建置順序最後載入它。

```sh
mkdir -p user-assets
cp examples/user-config.json user-assets/fish-config.json
chmod 600 user-assets/fish-config.json
# 在本機編輯上述檔案，勿把真實內容貼到聊天或 Issue。
python3 tools/prepare_user_config.py \
  --input user-assets/fish-config.json \
  --output user-assets/sdkconfig.local
```

私有設定範例中的值都是佔位符：

```json
{
  "tts_enabled": true,
  "api_key": "YOUR_OWN_API_KEY",
  "voice_id": "YOUR_OWN_VOICE_ID",
  "model": "s2.1-pro-free"
}
```

在已組裝的 SDK `esp32/` 目錄，用**新的**建置目錄；最後一項改成上述私人設定的絕對路徑：

```sh
idf.py -B build-fish-private -DIDF_TARGET=esp32s3 \
  -DSDKCONFIG=build-fish-private/sdkconfig \
  '-DSDKCONFIG_DEFAULTS=sdkconfig.defaults;devices/sdkconfig.muse;devices/sdkconfig.muse-waveshare-s3-175c;devices/sdkconfig.pet-public;/absolute/path/to/user-assets/sdkconfig.local' \
  -DPROJECT_VER=0.1.0-private-tts build
```

工具本身不建置、不刷機、不連網。任何載入金鑰的編譯結果，包括 bin、ELF、MAP、header、sdkconfig 與完整 Flash 備份，全部視為私人資料。刷寫前先按 [設定保留](storage-safety.md) 完成自己的備份與分區核對，不直接執行整組 flash 指令。

### API 對照

目前裝置程式以 `POST https://api.fish.audio/v1/tts`，使用 Bearer 認證與 `model` header；JSON 帶回覆 `text`、對應 `reference_id`、MP3 格式、32 kHz、64 kbps。這些音訊設定符合目前官方 API，亦與現有裝置解碼流程相接。`language=zh-TW` 是現用請求值；官方說明 language 用於文字正規化，不保證改變口音或音色。

`voice_id` 是本機設定名稱，發送時映射成 API 的 `reference_id`，不是把 API key 當 voice ID。聲音 ID 雖不是 API 認證金鑰，也可能連結個人的聲音模型，請私人保存。

模型／帳號、用量與費用請在服務端自行確認。API 回報認證、額度或服務錯誤時，保留字幕並顯示不含原始錯誤內容的提示；不要無限自動重試，也不要將服務回覆 body 直接公開。這是建議的錯誤呈現方式，完整裝置提示仍待實作與驗收。

## 逐步驗收

先以自己的帳號在服務端確認聲音，再用短句進行私人裝置測試，避免一開始朗讀完整私人對話。

- 確認字幕、正確聲音與喇叭音量，測試中文及中英混合。
- 測試打斷朗讀、下一回合、斷網恢復，以及服務失敗時是否仍能看字幕。
- 確認重新編譯的公版預設仍是空白憑證、TTS 關閉；私有 build 不加入來源 ZIP 或 Release。

這份文件沒有發出真實合成請求，也沒有啟用作者憑證。

## 方案 B：後端代理（後續提案）

資料路徑：ESP32 → 自己的 HTTPS `/tts` → Fish Audio → MP3 串流回裝置。後端保存 Fish key；每位使用者的聲音設定由後端以自己的識別資料映射，避免把長效供應商 key 放進共用 app。

需新增裝置端 TTS provider／代理 URL 與認證實作；**現有設定工具不支援填代理 URL，不能只改 API key 就切換。**

代理需具備：使用者／設備認證、可撤銷憑證、文字長度與用量限制、逾時與取消、限制音訊大小、供應商錯誤轉譯，以及不記錄原文／金鑰的日誌。設備憑證仍需保護；不要部署無認證、替所有人付費的公開轉送端點。代理也會接觸朗讀文字，不能宣稱資料完全留在設備。

## 建議對外介紹

> TamaMe 支援選配 Fish Audio 語音合成，作者私人版本已親測有效。公開版預設提供字幕與本機互動音效；要讓寵物朗讀回覆，請使用自己的 Fish Audio 帳號及有權使用的聲音，依串接文件自行設定與建置。專案不提供作者聲音、共用金鑰或代付額度。
