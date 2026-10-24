# 最小公開來源與依賴策略

> English summary: Describes pinned upstream assembly, reviewed overlays, dependencies, licences and versioned wardrobe storage. Full guide below is in Traditional Chinese.

採固定上游 SDK 加上明確來源 overlay，避免把整個私人工作資料夾作為開源輸入。

上游名稱：facebookincubator/muse-gadget-sdk。本機核對的基準 commit 為 e6fe5174092702ffddae6f90759a38d55a4ebae1。來源與雜湊清單見 source-plan.json；不包含作者 Git 歷史或遠端認證。

firmware-overlay/ 是本機功能修改與去識別後的檔案，保持 esp32/ 相對路徑。它不是整份 SDK：未修改的開機、Link／配對、板級、播放器、第三方組件與 CMake 支援仍來自固定上游。

## 後續組裝方式

1. 另外取得固定 commit 的乾淨上游來源；不要直接在作者現用 checkout 上組裝。
2. 審核 source-plan.json 的來源與每個 overlay 檔案雜湊，只套用清單內檔案。
3. 為 avatar/ 放入已確認且可散布的 portrait_assets.h、style_silhouette.h、avatar_profile.h、style_scene_assets.h。概念美術板不能替代它們。
4. ESP-IDF 6.0.1，以全新 build 目錄按設定文件載入 defaults。只做 build，刷機另依使用者指示執行。
5. 完成依賴解析、來源掃描、乾淨編譯、所有 UI 狀態及硬體驗證後，才製作發行包。

第四階段已用正式公版素材在獨立來源完成組裝與 ESP-IDF 建置；source-plan.json 仍不是硬體或發布驗收憑證。

## 依賴

已保留上游 main 及 Muse 的 idf_component.yml 宣告，包含 LVGL、codec、LVGL adapter、1.75C BSP 與 ESP-IDF 元件。dependencies-observed.json 是早期盤點參考；dependencies-resolved.json 記錄第四階段公開設定實際解析的 24 項版本與雜湊。

部分宣告仍支援其他板型，並非公開版已逐板驗證。首版目標只限 Waveshare ESP32-S3-Touch-AMOLED-1.75C。第四階段已完成乾淨建置、依賴與元件登錄及授權文本整理；完整人工權利審查仍待發布前確認。

目前字型沿用已整理的 Noto Sans TC 子集原始碼與 OFL 授權檔，沒有把介面封面的展示字型帶進韌體。根程式碼採 Apache-2.0，可授權的公版生成美術採 CC0-1.0；第三方與字型保留各自授權，最終逐項審查仍不可省略。

## 儲存設定

黑底／健身房／書桌旁／客廳／房間／咖啡廳固定為 0–5。換裝資料使用 pet_public_v2 namespace 與 schema 2，舊私人版設定不讀取、不清除、不遷移。未知版本或範圍不正確的資料不覆寫；寫入成功才更新有效選擇，失敗保留前一次有效狀態。

這只隔離換裝選擇，不會替使用者清除 Wi-Fi、配對或對話服務上的資料；重新刷入 app 不等於全機去識別。

公版寵物進度另外使用 pet_pub_stats，不更新原版 muse_pet_v1。音量、Wi-Fi 與配對等共用設定不在這項隔離範圍；見 storage-safety.md。
