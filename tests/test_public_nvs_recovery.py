"""Exercise the actual initializer: failures must never erase or write NVS."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class NvsRecoveryTests(unittest.TestCase):
    def test_preserves_storage_on_errors_and_loads_existing_settings(self):
        source = (ROOT/'firmware-overlay/esp32/components/muse/muse_settings.c').read_text()
        start = source.index('esp_err_t muse_settings_init(void)')
        end = source.index('\n}\n', start) + 3
        initializer = source[start:end]
        harness = r'''
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef int esp_err_t;
#define ESP_OK 0
#define ESP_ERR_NVS_NO_FREE_PAGES 1
#define ESP_ERR_NVS_NEW_VERSION_FOUND 2
#define NVS_READWRITE 1
#define NS "muse"
#define MUSE_MIC_GAIN_MAX 36
#define ESP_LOGE(...) ((void)0)
#define ESP_LOGI(...) ((void)0)
static struct {uint8_t volume;bool speaker_on;uint8_t mic_gain,brightness;uint16_t sleep_s;bool wifi_on,ble_on;char ssid[33],pass[65],host[64],vm[64],token[1024];} s;
static int s_lock,s_nvs,init_error,open_error,init_calls,opens,loads,writes,erases;
static int nvs_flash_init(void){init_calls++;return init_error;}
/* Abort even if a future change reintroduces erase or save calls. */
int nvs_flash_erase(void){erases++;assert(0);return 0;}
int nvs_set_str(int h,const char*k,const char*v){(void)h;(void)k;(void)v;writes++;assert(0);return 0;}
int nvs_commit(int h){(void)h;writes++;assert(0);return 0;}
static int xSemaphoreCreateMutex(void){return 7;}
static int nvs_open(const char*name,int mode,int*out){assert(!strcmp(name,"muse")&&mode==NVS_READWRITE);opens++;*out=8;return open_error;}
static int clampi(int x,int lo,int hi){return x<lo?lo:x>hi?hi:x;}
static void load_u8(const char*k,uint8_t*out){loads++;*out=!strcmp(k,"volume")?55:30;}
static void load_str(const char*k,char*out,size_t n){loads++;assert(n>20);strcpy(out,!strcmp(k,"token")?"synthetic-pair-token":"synthetic-config");}
static int nvs_get_u8(int h,const char*k,uint8_t*out){assert(h==8);(void)k;loads++;*out=1;return 0;}
static int nvs_get_u16(int h,const char*k,uint16_t*out){assert(h==8&&!strcmp(k,"sleep_s"));loads++;*out=120;return 0;}
'''
        harness += initializer + r'''
int main(void){
 int errors[]={ESP_ERR_NVS_NO_FREE_PAGES,ESP_ERR_NVS_NEW_VERSION_FOUND,99};
 for(unsigned i=0;i<sizeof errors/sizeof errors[0];i++){
  init_error=errors[i];init_calls=opens=loads=0;
  assert(muse_settings_init()==errors[i]);
  assert(init_calls==1&&opens==0&&loads==0&&writes==0&&erases==0);
 }
 init_error=0;open_error=42;assert(muse_settings_init()==42);assert(loads==0);
 open_error=0;assert(muse_settings_init()==ESP_OK);
 assert(s.volume==55&&s.sleep_s==120&&s.wifi_on);
 assert(!strcmp(s.token,"synthetic-pair-token"));assert(writes==0&&erases==0);
}
'''
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp)
            (p/'test.c').write_text(harness)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',
                            '-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],
                           check=True,capture_output=True,text=True)
            subprocess.run([str(p/'test')],check=True,capture_output=True,text=True)
