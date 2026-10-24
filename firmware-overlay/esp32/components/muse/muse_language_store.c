/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_language_store.h"
#include "nvs.h"
esp_err_t muse_language_load(void)
{
    muse_i18n_set_language(MUSE_LANG_EN);
    nvs_handle_t handle;
    esp_err_t err=nvs_open("tamame_ui",NVS_READONLY,&handle);
    if(err==ESP_ERR_NVS_NOT_FOUND)return ESP_OK;
    if(err!=ESP_OK)return err;
    uint8_t language=MUSE_LANG_EN;
    err=nvs_get_u8(handle,"language",&language);
    nvs_close(handle);
    if(err==ESP_ERR_NVS_NOT_FOUND)return ESP_OK;
    if(err==ESP_OK)muse_i18n_set_language(language); /* invalid values retain English */
    return err;
}
esp_err_t muse_language_save(unsigned language)
{
    if(language>=MUSE_LANG_COUNT)return ESP_ERR_INVALID_ARG;
    nvs_handle_t handle;
    esp_err_t err=nvs_open("tamame_ui",NVS_READWRITE,&handle);
    if(err!=ESP_OK)return err;
    err=nvs_set_u8(handle,"language",(uint8_t)language);
    if(err==ESP_OK)err=nvs_commit(handle);
    nvs_close(handle);
    return err; /* caller restarts only on successful commit */
}
