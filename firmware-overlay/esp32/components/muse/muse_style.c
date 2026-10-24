/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_style.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs.h"
#include "esp_log.h"
static QueueHandle_t commands,results;
static portMUX_TYPE lock=portMUX_INITIALIZER_UNLOCKED;
static muse_style_t saved={.version=2};
static bool ready,started;
static void publish(const muse_style_t *s){portENTER_CRITICAL(&lock);saved=*s;ready=true;portEXIT_CRITICAL(&lock);}
static void worker(void *arg){
 (void)arg;nvs_handle_t h;
 if(nvs_open("pet_public_v2",NVS_READWRITE,&h)!=ESP_OK){ESP_LOGE("muse_style","storage unavailable");vTaskDelete(NULL);return;}
 muse_style_t s={.version=2};size_t n=sizeof s;esp_err_t e=nvs_get_blob(h,"selection",&s,&n);
 if(e==ESP_ERR_NVS_NOT_FOUND){s=(muse_style_t){.version=2};}
 else if(e!=ESP_OK||n!=sizeof s||!muse_style_valid(&s)){ESP_LOGE("muse_style","invalid selection; storage left untouched");nvs_close(h);vTaskDelete(NULL);return;}
 publish(&s);ESP_LOGI("muse_style","ready bg=%u clothing=%u accessory=%u glasses=%u",s.background,s.clothing,s.accessory,s.sunglasses);
 for(;;){muse_style_t next;if(xQueueReceive(commands,&next,portMAX_DELAY)!=pdTRUE)continue;
  int result=-1;
  if(muse_style_valid(&next)&&nvs_set_blob(h,"selection",&next,sizeof next)==ESP_OK&&nvs_commit(h)==ESP_OK){publish(&next);result=1;ESP_LOGI("muse_style","selection saved");}
  else ESP_LOGE("muse_style","save failed");
  xQueueSend(results,&result,portMAX_DELAY);
 }
}
void muse_style_start(void){if(started)return;started=true;commands=xQueueCreate(1,sizeof(muse_style_t));results=xQueueCreate(1,sizeof(int));if(!commands||!results||xTaskCreate(worker,"muse_style",4096,NULL,2,NULL)!=pdPASS)ESP_LOGE("muse_style","worker unavailable");}
bool muse_style_snapshot(muse_style_t *out){portENTER_CRITICAL(&lock);bool ok=ready;*out=saved;portEXIT_CRITICAL(&lock);return ok;}
bool muse_style_save(const muse_style_t *s){muse_style_t ignored;return muse_style_valid(s)&&muse_style_snapshot(&ignored)&&commands&&xQueueSend(commands,s,0)==pdTRUE;}
int muse_style_feedback(void){int result=0;if(results)xQueueReceive(results,&result,0);return result;}
