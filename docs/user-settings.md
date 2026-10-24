# 使用者設定與公開版預設

[English](user-settings.en.md) · [简体中文](user-settings.zh-CN.md) · [繁體中文](user-settings.md)

公開版預設不帶帳號、不帶聲音 ID、不帶 Wi-Fi、不開啟外部 TTS 或手機攝影機。這不代表 Muse 對話可以離線運作；AI 對話仍需以自己的帳號配對服務。

## 本機設定範例

在公開候選目錄中執行，先把 examples/user-config.json 複製到 user-assets/my-config.json。預設 tts_enabled=false，金鑰與聲音 ID 都留空。

```sh
mkdir -p user-assets
cp examples/user-config.json user-assets/my-config.json
python3 tools/prepare_user_config.py --input user-assets/my-config.json --output user-assets/sdkconfig.local
```

工具只在本機產生權限 0600 的設定檔，不刪除輸入、不修改 SDK、不建置、不刷機、不連網。既有輸出不覆寫。請把輸入也留在私人目錄。

若選用 Fish Audio，將 tts_enabled 改成 true，再填入自己的 api_key、voice_id，model 則依服務帳號實際支援值設定；範例模型名稱來自目前程式設定，不保證帳號可用性。聲音需有權使用，不提供作者聲音或任何聲音複製素材。

停用時工具拒絕殘留金鑰；啟用時缺少金鑰或聲音 ID 也會拒絕產生設定。不要把真實設定貼進 Issue、社群或 AI 對話中。

## 建置前必讀

第四階段已完成預設關閉 TTS 的公開設定建置，尚未驗證真實服務。載入設定的順序為：SDK 預設 → Muse 預設 → 1.75C 硬體預設 → sdkconfig.pet-public → 自己的私人設定（選配）。必須使用新建置目錄，不能沿用私人版 sdkconfig。

選用 TTS 的金鑰和聲音 ID 會編譯進應用程式，這種建置結果必須視為私人檔案；不要分享它的 bin、ELF、MAP、產生的 header 或 sdkconfig。公開可下載韌體只能從乾淨、未帶憑證的設定重新建置。

Muse SDK Token 保留由使用者自行在設備／配對流程設定；本工具不輸入、不保存、不複製此 Token。Wi-Fi 與配對資料不從作者設備備份取得。

## 缺少設定時

- TTS 停用：不啟動本模組的外部 TTS 或附件音訊下載 worker；已有字幕流程保留。
- TTS 已開但金鑰／聲音 ID 不完整：文字 TTS 不啟動。附件音訊的 GET 是獨立流程，仍可能由回覆觸發，因此不應以「沒填 TTS 金鑰」推定沒有任何網路請求。
- 服務錯誤：本機模擬驗證了 worker 失敗與回收；公版完整 TTS 的錯誤提示與聽感仍待驗證。

不能把可見字幕的程式路徑或主機 mock 測試，當作真實 TTS 播放已通過驗證。本機互動音效已由單台實機使用者另行確認，詳見 [驗證狀態](validation-status.md)。

## Fish Audio 建議串接

作者私人版本已親測 Fish Audio 有效；公版預設不帶憑證、不開啟朗讀。完整操作、API 對照、私人建置與後端代理提案見 [Fish Audio 串接方案](fish-audio.md)。私人版本的成功不等於公版預設 TTS 已驗收。

## 優先字型支援進度

公版預設將繁簡聯集字型編譯進韌體，涵蓋 Big5 一級、GB2312 一級、目前 UI 用字及既有符號；合計 7,379 碼位，無須事後安裝字型。此階段不翻譯字幕或 Muse 回覆。English／简体中文／繁體中文設定切換與統一字串表已實作；預設英文，保存至公開版 NVS 後重新啟動生效。稀有字、emoji 與其他文字系統不在保證範圍。見 [字型範圍與測試](font-coverage.md)（英文）。

## 介面語言

開啟設定並捲動至語言區，選擇 English、简体中文或繁體中文。選擇會存入公開版 NVS 命名空間 `tamame_ui` 的 `language`，然後重新啟動以一致重建所有頁面。保存失敗時保留目前介面且不重啟。新裝置、缺少設定或無效值預設英文；讀取不會清除儲存資料。此設定只更改固定介面與每日運勢，不翻譯 Muse 回覆、辨識字幕、Wi-Fi 名稱，也不更改 TTS 聲音。重啟會清除暫存的最近訊息，配對、網路與換裝設定保留。完整轉讓／清除流程仍待驗收；語言切換不是恢復原廠設定。

## 換裝飾品

換裝分為背景、服裝、飾品三個分類。配件擇一或選「不加配件」；墨鏡可獨立勾選，與配件同時佩戴。「不加配件」只清除配件，保留墨鏡狀態；再點墨鏡即可單獨取下。選擇立即預覽，套用保存兩種設定，取消回復已保存穿搭。既有配件編號與墨鏡旗標保留，無需遷移或重設穿搭。背景選項可上下捲動。
