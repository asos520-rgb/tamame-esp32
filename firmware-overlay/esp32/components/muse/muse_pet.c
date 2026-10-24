/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_pet.h"
#include "muse_mem.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/idf_additions.h"
#include "nvs.h"
#include <time.h>
#ifdef ESP_PLATFORM
#include "esp_netif_sntp.h"
static bool s_sntp_started;
static void clock_start(void) {
    if(s_sntp_started)return;
    esp_sntp_config_t config=ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.wait_for_sync=false; /* Network acquisition never blocks the pet worker. */
    esp_err_t err=esp_netif_sntp_init(&config);
    if(err==ESP_OK||err==ESP_ERR_INVALID_STATE){s_sntp_started=true;ESP_LOGI("muse_pet","automatic SNTP enabled");}
}
#else
static void clock_start(void){}
#endif
#include <string.h>
static QueueHandle_t s_commands,s_feedback;
static portMUX_TYPE s_lock=portMUX_INITIALIZER_UNLOCKED;
static muse_pet_model_t s_visible;
static bool s_ready,s_started;
static void publish(const muse_pet_model_t *s) {
    portENTER_CRITICAL(&s_lock);s_visible=*s;s_ready=true;portEXIT_CRITICAL(&s_lock);
}
static bool store(nvs_handle_t h,const muse_pet_model_t *s) {
    return nvs_set_blob(h,"state",s,sizeof(*s))==ESP_OK&&nvs_commit(h)==ESP_OK;
}
static void worker(void *arg) {
    (void)arg;nvs_handle_t h;
    if(nvs_open("pet_pub_stats",NVS_READWRITE,&h)!=ESP_OK){ESP_LOGE("muse_pet","storage unavailable");vTaskDelete(NULL);return;}
    muse_pet_model_t state;size_t bytes=sizeof(state);
    esp_err_t err=nvs_get_blob(h,"state",&state,&bytes);
    if(err==ESP_ERR_NVS_NOT_FOUND){
        muse_pet_model_init(&state,time(NULL));
        if(!store(h,&state)){ESP_LOGE("muse_pet","initial save failed");nvs_close(h);vTaskDelete(NULL);return;}
    }
    else if(err!=ESP_OK||bytes!=sizeof(state)||!muse_pet_model_valid(&state)){
        /* Unknown/corrupt schemas are never silently reset: avoid duplicating rewards. */
        ESP_LOGE("muse_pet","invalid storage; rewards disabled");nvs_close(h);vTaskDelete(NULL);return;
    }
    muse_pet_model_decay(&state,time(NULL));
    publish(&state);unsigned ticks=0;int64_t mono=esp_timer_get_time(),uncalibrated=0;
    for(;;) {
        clock_start();
        muse_pet_action_t action;bool command=xQueueReceive(s_commands,&action,pdMS_TO_TICKS(30000))==pdTRUE;
        muse_pet_model_t candidate=state;int64_t now=time(NULL);
        int64_t next_mono=esp_timer_get_time();
        if(now<MUSE_PET_MIN_TIME){
            uncalibrated+=next_mono-mono;
            uint32_t seconds=(uint32_t)(uncalibrated/36000000)*36;
            if(seconds){muse_pet_model_decay_uncalibrated(&candidate,seconds);uncalibrated-=(int64_t)seconds*1000000;}
        }else uncalibrated=0;
        mono=next_mono;
        muse_pet_result_t result=command?muse_pet_model_apply(&candidate,action,now,esp_random()):PET_OK;
        if(!command)muse_pet_model_decay(&candidate,now);
        bool changed=memcmp(&candidate,&state,sizeof(state))!=0;
        /* Rewards and spending commit atomically before publishing success.
         * Passive decay saves at most every 30 minutes, not every frame. */
        bool bound_day=(state.reward_day<0&&candidate.reward_day>=0)||(state.fortune_day==0&&candidate.fortune_day>0);
        bool persist=command||bound_day||++ticks>=60;
        if(persist) {
            ticks=0;
            if(!store(h,&candidate)){result=PET_STORAGE;candidate=state;}
        }
        if(result!=PET_STORAGE){state=candidate;if(changed)publish(&state);}
        if(command&&action!=PET_CONVERSATION) {
            muse_pet_feedback_t feedback={action,result,state.fortune};
            /* No UI/display lock here. Request budget <= result queue capacity. */
            xQueueSend(s_feedback,&feedback,portMAX_DELAY);
        }
    }
}
void muse_pet_start(void) {
    if(s_started)return;
    s_started=true;
    s_commands=xQueueCreate(8,sizeof(muse_pet_action_t));s_feedback=xQueueCreate(8,sizeof(muse_pet_feedback_t));
    /* Internal stack: NVS commits disable cache, so PSRAM stacks are unsafe here. */
    if(!s_commands||!s_feedback||xTaskCreate(worker,"muse_pet",4096,NULL,2,NULL)!=pdPASS) {
        ESP_LOGE("muse_pet","worker unavailable");
    }
}
bool muse_pet_request(muse_pet_action_t action) {
    portENTER_CRITICAL(&s_lock);bool ready=s_ready;portEXIT_CRITICAL(&s_lock);
    return ready&&s_commands&&xQueueSend(s_commands,&action,0)==pdTRUE;
}
bool muse_pet_snapshot(muse_pet_model_t *out) {
    portENTER_CRITICAL(&s_lock);bool ready=s_ready;*out=s_visible;portEXIT_CRITICAL(&s_lock);return ready;
}
bool muse_pet_feedback(muse_pet_feedback_t *out) {return s_feedback&&xQueueReceive(s_feedback,out,0)==pdTRUE;}
void muse_pet_conversation_completed(void){(void)muse_pet_request(PET_CONVERSATION);}
