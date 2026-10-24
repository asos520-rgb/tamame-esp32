# 本機來源組裝與建置預覽

[English](build-preview.en.md) · [简体中文](build-preview.zh-CN.md) · [繁體中文](build-preview.md)

目標只有 Waveshare ESP32-S3-Touch-AMOLED-1.75C，ESP-IDF 6.0.1。已在獨立目錄編譯成功，並完成單台設備的首輪畫面、背景切換重測、重啟保存、字幕與本機音效確認；仍有未驗證流程，詳見 [驗證狀態](validation-status.md)。

## 來源組裝

先自行取得 Muse Gadget SDK。工具只讀 source-plan.json 固定 commit 的 Git 物件，不讀 checkout 中的私密修改、未追蹤檔案或 sdkconfig，也不複製 Git 歷史。

在此預覽套件根目錄執行；將路徑替換成自己的 SDK 儲存庫位置：

```sh
mkdir -p user-assets
python3 tools/assemble_sdk.py --upstream /path/to/muse-gadget-sdk --output user-assets/public-sdk
```

工具先核對來源與四份必要素材，不覆寫既有目錄、不下載、不建置、不刷機。後續需自行安裝／啟用 ESP-IDF。首次解析管理元件可能需要連網；既有第三方快取不是作者設定，仍應核對 lock 與授權。

## 乾淨建置

在新組裝的 public-sdk/esp32 目錄執行，先啟用自己的 ESP-IDF 6.0.1 環境：

```sh
idf.py -B build-public -DIDF_TARGET=esp32s3 \
  -DSDKCONFIG=build-public/sdkconfig \
  '-DSDKCONFIG_DEFAULTS=sdkconfig.defaults;devices/sdkconfig.muse;devices/sdkconfig.muse-waveshare-s3-175c;devices/sdkconfig.pet-public' \
  -DPROJECT_VER=0.1.0-public-preview build
```

不得沿用私人建置目錄。公開預覽不帶 SDK Token、Wi-Fi、配對資料或 TTS 憑證；編譯會提示 SDK Token 空白。真實配對／服務可用性仍待驗證，使用者需要依實際服務流程設定自己的 Token。

若要減少編譯產物包含本機路徑，可額外傳入 C/C++ 的 -ffile-prefix-map，將自己的來源根目錄映射到通用 build 路徑。請在本機設定，不要把真實主目錄寫進公開說明或提交。

本輪產物檢查：應用映像 3,280,896 bytes，4,194,304-byte app 分區剩餘 913,408 bytes（約 21.78%）；實際數值記於本機驗證報告。未發布可下載的韌體 bin／ELF／NVS 備份。

## 驗證界線

已驗證正式素材格式、實際 C renderer 的狀態矩陣、原生記憶體檢查、設定與 TTS mock，以及 ESP-IDF 編譯。docs/images/public-render-review-v2.png 是實際渲染圖，不是實機截圖。

尚待：完整觸控與互動矩陣、全新配對、TTS、缺少帳號與語音設定時的提示、完整日誌審查、長時間穩定性及清除資料流程。不要直接執行上游列出的全區 flash／erase 命令；本預覽工具不會操作設備。
