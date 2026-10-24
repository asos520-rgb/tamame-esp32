# TamaMe 發布前清單

> English summary: Lists publication prerequisites, identity and security-channel decisions, validation gaps and a draft source-preview release description. Full guide below is in Traditional Chinese.

建議儲存庫名稱 `tamame-esp32`，產品名稱已選定為 TamaMe。尚未建立／指定遠端，這不是名稱可用性或商標核准。

## 已有

- TamaMe 中英文首頁、固定上游來源、素材與授權通知。
- 角色轉換與私人設定工具、Fish Audio 串接方法、來源建置說明。
- 第一輪實機與完整回退證據，明確標示已知限制。
- 問題／PR 模板、貢獻、支援及安全回報政策草案。

## 對外發布前仍需完成

- 選 owner、公開署名及提交 Email；不沿用電腦本機帳號或私人信箱。
- 建立並驗證私密漏洞回報管道，更新 SECURITY.md；沒有管道時保持未發布。
- 再核對第三方授權、字型、音效／圖示與素材來源，以及名稱可用性；保留必要通知。
- 補上裝置端「語音尚未設定，目前以字幕顯示」提示，並驗證畫面；本輪只提供文案與說明。
- 明確決定全新配對、轉讓清除及已知背景異常要先驗證，或在來源預覽標示為不支援／未驗證；不得稱正式即用版。
- 從本輪精確白名單組裝來源包，核對檔案與 SHA-256，不用整個本機專案建立 Git 歷史。
- 準備公版人物橫式封面、About／Topics 與草稿 Release；不帶作者真人形象或私人聲音。

## 發布內容草案

About：A customizable ESP32 desktop pixel pet based on Muse Gadget SDK, with a fictional default avatar and optional Fish Audio TTS.

Topics 候選：esp32、esp32-s3、desktop-pet、pixel-art、lvgl、tts。

初版建議標為 `v0.1.0-alpha.1` 的來源預覽，列出驗證板型、已測功能、已知背景問題、TTS 自行配置需求及未驗證流程。可附公開來源 ZIP 與 SHA-256；不附私人 bin、備份、ELF／MAP、錄音或原始日誌。

草稿介紹：

> TamaMe 是可自訂形象的 ESP32 桌面像素寵物，提供公版人物、通用場景、換裝與互動功能。首輪已驗證部分實機流程及完整備份回退；本次為來源預覽，仍有待驗證項目。Fish Audio 已在作者私人版本親測有效，公版預設關閉 TTS，請依串接說明使用自己的帳號與有權使用的聲音。
