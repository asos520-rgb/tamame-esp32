"""Compile the staged TTS source with mock HTTP; never calls a real service."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
COMP=ROOT/'firmware-overlay/esp32/components/muse'
FIXTURES=ROOT/'tests/fixtures'
JSON=Path(os.environ.get('CJSON_SOURCE_DIR','/nonexistent-cjson'))

class PublicTTSTests(unittest.TestCase):
    def prepare(self,d,config):
        for name in ['esp_crt_bundle.h','esp_heap_caps.h','esp_http_client.h','esp_timer.h','esp_log.h',
                     'freertos/FreeRTOS.h','freertos/idf_additions.h','freertos/task.h']:
            p=d/name;p.parent.mkdir(parents=True,exist_ok=True)
            p.write_text('#include "fish_tts_host_platform.h"\n')
        (d/'sdkconfig.h').write_text(config)
    def run_native(self,d,source,json_source=False):
        (d/'test.cpp').write_text(source)
        flags=['c++','-std=c++17','-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer',
               '-I',str(d),'-I',str(FIXTURES),'-I',str(COMP)]
        extras=[]
        if json_source:
            subprocess.run(['cc','-c',str(JSON/'cJSON.c'),'-I',str(JSON),'-o',str(d/'json.o')],check=True,capture_output=True)
            flags+=['-I',str(JSON)];extras=[str(d/'json.o')]
        else:(d/'cJSON.h').write_text('/* Disabled build must need no JSON functions. */\n')
        r=subprocess.run(flags+[str(d/'test.cpp')]+extras+['-o',str(d/'test')],capture_output=True,text=True)
        self.assertEqual(r.returncode,0,r.stderr)
        r=subprocess.run([str(d/'test')],capture_output=True,text=True,timeout=20,
                         env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})
        self.assertEqual(r.returncode,0,r.stderr)
    @unittest.skipUnless(shutil.which('c++'),'native compiler unavailable')
    def test_disabled_tts_has_no_http_worker(self):
        with tempfile.TemporaryDirectory() as tmp:
            d=Path(tmp);self.prepare(d,'#define CONFIG_MUSE_FISH_TTS 0\n')
            self.run_native(d,'''#include <cassert>
#include "fish_tts_host_platform.h"
#include "muse_fish_tts.cpp"
int main(){assert(!muse_fish_tts_configured());
assert(muse_fish_tts_start("synthetic message",1)==-1);
assert(muse_audio_url_start("https://example.com/test.mp3",1)==-1);
assert(workers.empty()&&request_url.empty());}
''')
    @unittest.skipUnless(shutil.which('c++') and (JSON/'cJSON.c').is_file(),'CJSON_SOURCE_DIR and compiler required')
    def test_enabled_without_credentials_has_no_tts_request(self):
        with tempfile.TemporaryDirectory() as tmp:
            d=Path(tmp);self.prepare(d,'#define CONFIG_MUSE_FISH_TTS 1\n#define CONFIG_MUSE_FISH_API_KEY ""\n#define CONFIG_MUSE_FISH_VOICE_ID ""\n#define CONFIG_MUSE_FISH_MODEL "example-model"\n')
            self.run_native(d,'''#include <cassert>
#include "fish_tts_host_platform.h"
#include "muse_fish_tts.cpp"
int main(){assert(!muse_fish_tts_configured());
assert(muse_fish_tts_start("synthetic message",1)==-1);
assert(workers.empty()&&request_url.empty());}
''',True)
    @unittest.skipUnless(shutil.which('c++') and (JSON/'cJSON.c').is_file(),'CJSON_SOURCE_DIR and compiler required')
    def test_opt_in_mock_stream_errors_cancel_and_no_auth_on_media(self):
        with tempfile.TemporaryDirectory() as tmp:
            d=Path(tmp);self.prepare(d,'#define CONFIG_MUSE_FISH_TTS 1\n#define CONFIG_MUSE_FISH_API_KEY "test_api_key"\n#define CONFIG_MUSE_FISH_VOICE_ID "test_voice"\n#define CONFIG_MUSE_FISH_MODEL "s2.1-pro-free"\n')
            self.run_native(d,(FIXTURES/'fish_tts_host_harness.cpp').read_text(),True)
