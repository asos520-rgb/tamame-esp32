# 公開候選資料流盤點

> English summary: Maps microphone, conversation, recent-reply, TTS, network and diagnostic data paths, including privacy limits and public defaults. Full guide below is in Traditional Chinese.

資料路徑依本機程式碼整理，不代表遠端服務的保存政策。後續首輪實機測試曾使用既有配對確認對話字幕；公開資料不包含真實對話或憑證，TTS 預設停用。

| 資料 | 本機流程與可能去向 | 公開版處理 |
|---|---|---|
| 麥克風收音 | 語音流程透過 Muse 連線傳送；目前程式有 /api/voice/dictation 與 /chat/stream 路徑 | 須由使用者配對，不能宣稱語音只在設備處理 |
| 輸入文字／辨識結果 | /chat/stream，回覆由 /chat/subscribe 等串流處理 | 不把辨識文字寫入候選 chat／voice 日誌 |
| 最近回覆 | muse_recent 保存最多八則收到的文字，使用 RAM／PSRAM，非 NVS 寫入 | 沒有作者對話 fixture；本機重啟會重新初始化，遠端保存另計 |
| 語音合成文字 | 選配 HTTPS POST 到 api.fish.audio/v1/tts，帶回覆文字、聲音 ID、模型及 Authorization | 預設關閉；憑證來自使用者自己的私人建置設定 |
| 回覆音訊附件 | 同一播放器可能以 HTTPS GET 下載回覆媒體 URL | 停用該模組時不啟動 worker；啟用時與 TTS 金鑰是否完整分開判斷；不附加 Fish 或設備憑證 |
| 角色與服裝選擇 | 本機模板素材與公開版 NVS 設定 | 獨立 namespace pet_public_v2、schema 2；不讀舊私人版選擇 |
| Wi-Fi／配對 | 上游 Link、BLE、設定頁與設備 NVS | 不複製作者 NVS；設定頁仍需顯示使用者自己的連線資訊，截圖需自行遮蔽 |
| 手動診斷 | 有 /chat/history?limit=3 和 /api/session/list 的探測流程 | 僅 schema／計數摘要，非自動同步完整歷史；其開關與端到端行為仍需整合驗證 |

## 已修改的診斷面

候選 muse_chat_session.cpp 與 muse_voice.c 的 ESP_LOG 呼叫改成固定事件字串，不輸出文字、ID、主機名或遠端錯誤 body。原本 USB @chat 的任意文字／欄位輸出改成固定 event；這會改變依賴序列回覆文字的外部工具行為，螢幕字幕路徑未在此函式修改。

USB status 回覆改為固定 privacy=redacted，不輸出 SSID、IP、VM 或設備身份；SDK Token 的設定頁預覽改成已設定狀態，不顯示前綴。原本未知 face 指令的回顯也改成固定錯誤文字。

這些修改不等於整份 SDK 的所有日誌已完成隱私稽核。上游未納入 overlay 的配對、連線、崩潰資訊與第三方函式庫仍須在第四階段審核；請勿分享原始完整序列日誌。螢幕本身會呈現自己的對話與網路資料，錄影和截圖也須檢查。

## 未納入公開版預設

作者照片、聲音、私人記憶、地點、班表、Wi-Fi、Token、舊聊天、設備備份及原本工作歷史，均不作預設或範例。自適應不宣稱具備班表／定位自動控制功能。

Fish Audio 作者私人版本已親測有效；使用者直連方法與尚未實作的後端代理提案見 [串接方案](fish-audio.md)。
