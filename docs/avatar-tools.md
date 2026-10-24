# 本機角色轉換工具

> English summary: Explains local avatar validation and conversion tools; no photo-upload or on-device avatar-import flow is provided. Full guide below is in Traditional Chinese.

工具不連網、不做人臉辨識、不生成新角色。你需自行提供有權使用、符合模板的八張圖片。

## 準備

在此候選目錄中執行。需要 Python 3.10 以上；建議建立獨立環境，再安裝固定版本的 Pillow：

```sh
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install -r requirements-avatar.txt
mkdir -p user-assets/my-avatar
```

將 boot.png、idle.png、listening.png、thinking.png、speaking.png、happy.png、error.png、off.png 放入 user-assets/my-avatar/。參閱 [角色素材規格](avatar-contract.md)。

複製 examples/avatar-profile.json 到 user-assets/my-profile.json。只修改 skin、cheek、mouth 色碼，讓手部和表情疊圖與你的原圖一致；首版模板的座標不可任意更改。

## 轉換

```sh
python3 tools/avatar_pack.py --input user-assets/my-avatar --profile user-assets/my-profile.json --output user-assets/my-pack-v1
```

輸出路徑的上層必須已存在，目標資料夾必須尚未存在。修改後請用新的版本名稱，工具不覆寫舊結果。

| 輸出 | 用途 |
|---|---|
| portrait_assets.h | 共用 RGB565 palette 與八組索引圖 |
| style_silhouette.h | 與顏色獨立的透明遮罩，LSB-first |
| avatar_profile.h | 模板版本、眼／嘴／衣服範圍、膚色設定 |
| preview.png | 從實際 RGB565 資料重建的八圖預覽，四欄兩列 |
| pack.json | 固定格式與狀態說明，不記錄輸入路徑或原圖中繼資料 |

狀態順序固定：boot、idle、listening、thinking、speaking、happy、error、off。像素、palette 與遮罩資料合計 37,376 bytes，不含 C header、預覽、設定常數或編譯 padding。

即使輸出移除了原圖附帶資訊，仍可還原角色樣貌，預設必須留在 user-assets/。不要把私人輸出加入發布白名單。工具只攔截格式錯誤，不替你確認素材授權或去識別效果。

## 測試與整合狀態

```sh
python3 -B -m unittest discover -s tests -p 'test_*.py' -v
```

合成測試會在暫存資料夾製作簡單幾何圖，驗證轉換結果；有 C 編譯器時另使用 AddressSanitizer／UndefinedBehaviorSanitizer 驗證 renderer。無 C 編譯器會略過該項，請勿把略過視為通過。測試不使用真人照片或私人服務。

第四階段已將正式公版人物、五款場景與新版儲存設定接到固定 SDK 並完成編譯；不要直接覆蓋私人 SDK。硬體驗證尚未完成，參閱 build-preview.md。
