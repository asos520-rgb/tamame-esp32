"""Run the real NVS worker through commit failure and reboot with strict mocks."""
from pathlib import Path
import subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]/'firmware-overlay/esp32'
class PetStorage(unittest.TestCase):
    def test_atomic_spending_failed_commit_and_reboot(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'freertos').mkdir()
            fakes={
              'muse_mem.h':'',
              'sdkconfig.h':'#define CONFIG_SPIRAM 1\n',
              'esp_heap_caps.h':'#define MALLOC_CAP_SPIRAM 1\n#define MALLOC_CAP_8BIT 2\n',
              'esp_log.h':'#define ESP_LOGI(...) ((void)0)\n#define ESP_LOGE(...) ((void)0)\n',
              'esp_random.h':'#include <stdint.h>\nuint32_t esp_random(void);\n',
              'esp_timer.h':'#include <stdint.h>\nint64_t esp_timer_get_time(void);\n',
              'freertos/FreeRTOS.h':'''#pragma once
#include <stdint.h>
#include <stddef.h>
typedef int portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED 0
#define portENTER_CRITICAL(p) ((void)(p))
#define portEXIT_CRITICAL(p) ((void)(p))
#define pdTRUE 1
#define pdPASS 1
#define portMAX_DELAY 0xffffffffu
#define pdMS_TO_TICKS(x) (x)
''',
              'freertos/queue.h':'''#pragma once
#include <stddef.h>
typedef void *QueueHandle_t;
QueueHandle_t xQueueCreate(unsigned,unsigned);
int xQueueSend(QueueHandle_t,const void*,unsigned);
int xQueueReceive(QueueHandle_t,void*,unsigned);
''',
              'freertos/task.h':'void vTaskDelete(void*);\nint xTaskCreate(void (*)(void*),const char*,unsigned,void*,unsigned,void*);\n',
              'freertos/idf_additions.h':'',
              'nvs.h':'''#pragma once
#include <stddef.h>
typedef int nvs_handle_t;
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_NVS_NOT_FOUND 1
#define NVS_READWRITE 1
int nvs_open(const char*,int,nvs_handle_t*);
int nvs_get_blob(nvs_handle_t,const char*,void*,size_t*);
int nvs_set_blob(nvs_handle_t,const char*,const void*,size_t);
int nvs_commit(nvs_handle_t);
void nvs_close(nvs_handle_t);
'''}
            for name,s in fakes.items():(p/name).write_text(s)
            source=r'''
#include "muse_pet.h"
#include "nvs.h"
#include <assert.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>
static jmp_buf stop;
static muse_pet_model_t disk,staged;
static int commits,step,phase,feedback_count;
static muse_pet_feedback_t results[16];
static void (*task)(void*);
static muse_pet_action_t actions[]={PET_COFFEE,PET_COFFEE,PET_FORTUNE,PET_TOUCH,PET_SFX_OFF,PET_CONVERSATION};
time_t time(time_t *out){time_t t=1800000000;if(out)*out=t;return t;}
int64_t esp_timer_get_time(void){return step*1000;}
uint32_t esp_random(void){return 12;}
void *xQueueCreate(unsigned n,unsigned bytes){(void)n;static int id;(void)bytes;return (void*)(intptr_t)++id;}
int xQueueSend(void *q,const void *value,unsigned wait){
 (void)wait;assert(q==(void*)2);results[feedback_count++]=*(const muse_pet_feedback_t*)value;return 1;
}
int xQueueReceive(void *q,void *value,unsigned wait){
 (void)wait;assert(q==(void*)1);
 if(phase==0){if(step>=6)longjmp(stop,1);*(muse_pet_action_t*)value=actions[step++];}
 else {if(step>=1)longjmp(stop,1);step++;*(muse_pet_action_t*)value=PET_TOUCH;}
 return 1;
}
int xTaskCreate(void (*fn)(void*),const char *name,unsigned stack,void *arg,unsigned priority,void *handle){
 (void)arg;(void)handle;assert(!strcmp(name,"muse_pet")&&stack==4096&&priority==2);task=fn;return 1;
}
void vTaskDelete(void *p){(void)p;assert(0);}
int nvs_open(const char *name,int mode,nvs_handle_t *out){assert(!strcmp(name,"pet_pub_stats")&&mode==NVS_READWRITE);*out=7;return 0;}
int nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){assert(h==7&&!strcmp(key,"state")&&*size==sizeof disk);memcpy(out,&disk,sizeof disk);return 0;}
int nvs_set_blob(nvs_handle_t h,const char *key,const void *in,size_t size){assert(h==7&&!strcmp(key,"state")&&size==sizeof disk);memcpy(&staged,in,size);return 0;}
int nvs_commit(nvs_handle_t h){assert(h==7);commits++;if(commits==2)return -1;disk=staged;return 0;}
void nvs_close(nvs_handle_t h){assert(h==7);}
''' + f'\n#include "{ROOT/"components/muse/muse_pet.c"}"\n' + r'''
int main(void){
 muse_pet_model_init(&disk,1800000000);disk.coins=10;disk.hunger=50000;
 muse_pet_start();assert(task);
 if(!setjmp(stop))task(NULL);
 muse_pet_model_t visible;assert(muse_pet_snapshot(&visible));
 assert(visible.coins==8&&disk.coins==8&&visible.hunger==65000&&disk.hunger==65000);
 assert(results[0].result==PET_OK&&results[1].result==PET_STORAGE);
 assert(disk.fortune==12&&disk.daily_pets==1&&disk.sfx==0&&disk.energy==85000);
 assert(!memcmp(&visible,&disk,sizeof disk));
 /* Reboot worker: reload the committed blob; no reward/date/preferences reset. */
 phase=1;step=0;
 if(!setjmp(stop))task(NULL);
 assert(muse_pet_snapshot(&visible));assert(visible.coins==9&&visible.daily_pets==2&&visible.fortune==12&&visible.sfx==0);
 assert(!memcmp(&visible,&disk,sizeof disk));
}
'''
            (p/'test.c').write_text(source)
            r=subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I',str(p),'-I',str(ROOT/'components/muse'),str(p/'test.c'),str(ROOT/'components/muse/muse_pet_model.c'),str(ROOT/'components/muse/muse_i18n.c'),'-o',str(p/'test')],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stderr)
            r=subprocess.run([str(p/'test')],capture_output=True,text=True);self.assertEqual(r.returncode,0,r.stderr)
