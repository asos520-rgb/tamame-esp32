# 參與 TamaMe

[English](CONTRIBUTING.md) · 简体中文（待补 / pending） · [繁體中文](CONTRIBUTING.zh-TW.md)

歡迎以合成資料提出可重現問題、文件修正與小範圍變更。先說明具體問題、預期行為及目標硬體，避免將個人設定或素材當作測試 fixture。

1. 使用固定上游與審核過的 overlay 組裝來源；不要將整個私人 checkout 或 Git 歷史搬入套件。
2. 在新分支作單一目的修改。新增／修改 overlay 後，同步更新 source-plan.json 中對應檔案 SHA-256；這不替代人工審查。
3. 功能修正執行相關原生測試；韌體變更完成目標板建置。硬體測試需說明板型、版本與實際驗證項目，沒有測過就標示未測。
4. 提供問題、變更、驗證與限制。變更素材時交代來源、修改範圍與散布權利，移除 PNG 附帶資訊及私人參考資料。

測試入口：`python3 -B -m unittest discover -s tests -p 'test_*.py'`。角色測試需要 Pillow 與本機 C/C++ compiler；TTS 測試需要以 CJSON_SOURCE_DIR 指向含 cJSON.c 的目錄，請檢查有無 skipped，不把略過視為通過。測試使用 mock，不需填真實金鑰。

勿提交 sdkconfig、user-assets、原始錄音、NVS、bin／ELF／MAP、完整日誌、配對憑證或私人角色／聲音。Git 提交請自行選用公開署名與非私人 Email；提交歷史也屬公開內容。

不要對他人的連線設備執行刷機、清除或配對操作。自動化測試不得預設呼叫付費 AI／TTS 服務。
