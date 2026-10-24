"""Run the real NVS worker through commit failure and reboot with strict mocks."""
from pathlib import Path
import subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]/'firmware-overlay/esp32'
class StyleStorage(unittest.TestCase):
    def test_atomic_style_save_failure_and_reboot(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp);(p/'freertos').mkdir()
            fakes={
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
            source='\n#include "muse_style.h"\n#include "nvs.h"\n#include <assert.h>\n#include <setjmp.h>\n#include <string.h>\nstatic jmp_buf stop;\nstatic muse_style_t disk={.version=2},staged;\nstatic int step,phase,commits,test_results[4],count;\nstatic void (*task)(void*);\nvoid *xQueueCreate(unsigned n,unsigned bytes){(void)n;(void)bytes;static int id;return (void*)(intptr_t)++id;}\nint xQueueSend(void *q,const void *v,unsigned wait){(void)wait;if(q==(void*)2)test_results[count++]=*(const int*)v;return 1;}\nint xQueueReceive(void *q,void *v,unsigned wait){(void)wait;assert(q==(void*)1);if(phase||step==3)longjmp(stop,1);muse_style_t s={2,5,2,2,1};if(step==1)s.background=4;if(step==2)s.clothing=99;step++;*(muse_style_t*)v=s;return 1;}\nint xTaskCreate(void (*fn)(void*),const char *name,unsigned stack,void *arg,unsigned priority,void *handle){(void)arg;(void)handle;assert(!strcmp(name,"muse_style")&&stack==4096&&priority==2);task=fn;return 1;}\nvoid vTaskDelete(void *p){(void)p;longjmp(stop,2);}\nint nvs_open(const char *name,int mode,nvs_handle_t *out){assert(!strcmp(name,"pet_public_v2")&&mode==NVS_READWRITE);*out=7;return 0;}\nint nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *size){assert(h==7&&!strcmp(key,"selection")&&*size==sizeof disk);memcpy(out,&disk,sizeof disk);return 0;}\nint nvs_set_blob(nvs_handle_t h,const char *key,const void *in,size_t size){assert(h==7&&!strcmp(key,"selection")&&size==sizeof disk);memcpy(&staged,in,size);return 0;}\nint nvs_commit(nvs_handle_t h){assert(h==7);commits++;if(commits==2)return -1;disk=staged;return 0;}\nvoid nvs_close(nvs_handle_t h){assert(h==7);}\n\n#include "muse_style.c"\n\nint main(void){\n muse_style_t s;assert(!muse_style_snapshot(&s));muse_style_start();assert(task);\n if(!setjmp(stop))task(NULL);\n assert(muse_style_snapshot(&s));assert(s.background==5&&s.clothing==2&&s.accessory==2&&s.sunglasses==1);\n assert(!memcmp(&s,&disk,sizeof disk));assert(test_results[0]==1&&test_results[1]==-1&&test_results[2]==-1&&commits==2);\n phase=1;if(!setjmp(stop))task(NULL);assert(muse_style_snapshot(&s));assert(!memcmp(&s,&disk,sizeof disk));\n for(int field=0;field<4;field++){muse_style_t invalid=s;((uint8_t*)&invalid)[4+field]=255;assert(!muse_style_valid(&invalid));assert(!muse_style_save(&invalid));}\n for(int bg=0;bg<=5;bg++){s.background=bg;assert(muse_style_valid(&s));}s.background=6;assert(!muse_style_valid(&s));s.background=0;s.version=1;assert(!muse_style_valid(&s));\n phase=2;disk.version=1;ready=false;int result=setjmp(stop);if(!result)task(NULL);assert(result==2&&!muse_style_snapshot(&s)&&disk.version==1&&commits==2);\n}\n'
            (p/'test.c').write_text(source)
            r=subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I',str(p),'-I',str(ROOT/'components/muse'),str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stderr)
            r=subprocess.run([str(p/'test')],capture_output=True,text=True);self.assertEqual(r.returncode,0,r.stderr)
