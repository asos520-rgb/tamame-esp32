# 待整合的公版 renderer

由既有 Muse pixel renderer 的本機修改版本整理，保留上游著作權標示，授權副本見 LICENSE.sdk。尚非完整韌體；不附原人物或原場景陣列。

本階段修改：

- 新增 avatar_profile.h 相依，集中模板版本、臉部／衣服範圍與膚色。
- 保留不透明黑色；表情局部 patch 同樣依來源 silhouette 判斷透明。
- 衣服移除品牌像素，保留運動服及圍裙疊圖。
- 場景範圍改為 0–5，對應五類公開版規格。

renderer/ 保留通用渲染原始碼；正式公版的四份素材 header 已放在 assets/default-v2/pack 與 firmware-overlay/esp32/components/muse/avatar。新手可直接使用這組可分享素材，不必先自行製圖。

測試以合成角色及純色場景產生這些檔案，在暫存目錄編譯；測試通過只表示這組相容介面及狀態渲染通過，不代表硬體已驗收；正式公版另有完整狀態矩陣測試與實際渲染預覽。muse_state.h 與 muse_pixel.h 保留上游介面，後續韌體整合需核對目前 ABI。
