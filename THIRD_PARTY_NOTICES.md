# 第三方來源與授權保留

SDK 基準與每份修改來源見 source-plan.json。根 LICENSE 是 Apache-2.0，NOTICE 列出來源；第三方與美術不因此被重新授權。

此次公開設定建置登錄了 169 個元件，其中 23 個來自管理元件。third-party-licenses/index.json 列出這些管理元件的授權文本位置；另保留 SDK 內建 minimp3、tinyh264 與 libvpx_vp8 的根授權／專利聲明。這是建置登錄清單，未把所有登錄元件說成最終映像都使用。

Noto Sans TC 子集的授權保留在 firmware-overlay/esp32/components/muse/fonts/NotoSansTC-OFL.txt。公版美術採 CC0，另見 assets/README.md。原上游 Jollybot 美術不屬根 Apache 授權；這個預覽的角色 header 使用新虛構人物，沒有把原上游美術重新標成 Apache。

組裝工具會帶入固定上游提交的原始來源及其既有授權，不複製作者個人工作歷史。上游各子目錄仍可能有個別通知與授權要求，發行時應連同原始來源保留；不能只保留本頁或根 LICENSE。

此來源預覽尚未做完整的人工逐檔權利審核或商標檢索，也沒有把真實服務條款視為程式碼授權。GitHub 正式發布前，仍需確認發行內容、上游來源與個別素材範圍。
