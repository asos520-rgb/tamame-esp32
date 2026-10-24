"""Compile the real catalog and NVS preference code; fail on incomplete translations."""
from pathlib import Path
import json,re,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
COMP=ROOT/'firmware-overlay/esp32/components/muse'
Q=r'"(?:\\.|[^"\\])*"'
def catalog():
    text=(COMP/'muse_i18n_catalog.def').read_text()
    keys={k:json.loads(v) for k,v in re.findall(r'#define MUSE_KEY_(\w+) ('+Q+r')',text)}
    rows=[(k,json.loads(en),json.loads(cn),keys[tw]) for k,en,cn,tw in re.findall(r'MUSE_STRING\((\w+), ('+Q+r'), ('+Q+r'), MUSE_KEY_(\w+)\)',text)]
    return text,keys,rows
class LanguageTests(unittest.TestCase):
    def test_all_keys_have_three_nonempty_translations_and_matching_formats(self):
        text,keys,rows=catalog()
        self.assertGreater(len(rows),300)
        self.assertEqual(set(keys),{r[0] for r in rows})
        self.assertEqual(len(rows),len(keys))
        self.assertEqual(len(keys.values()),len(set(keys.values())))
        fmt=r'%(?:[-+ #0]*\d*(?:\.\d+)?(?:hh|ll|[hljztL])?[diuoxXfFeEgGaAcspn]|%)'
        for key,*values in rows:
            self.assertTrue(all(values),key)
            signatures=[re.findall(fmt,v) for v in values]
            self.assertEqual(signatures[0],signatures[1],key)
            self.assertEqual(signatures[0],signatures[2],key)
            self.assertFalse(re.search('[\u3400-\u9fff]',values[0]),key) if key!='s320' else None
        for p in COMP.glob('*.c'):
            for used in re.findall(r'MUSE_KEY_(\w+)',p.read_text()):self.assertIn(used,keys,str(p))
    def test_actual_catalog_default_switch_invalid_value_and_all_lookups(self):
        _,_,rows=catalog()
        code='#include "muse_i18n.h"\n#include <assert.h>\n#include <string.h>\nint main(void){assert(muse_i18n_language()==MUSE_LANG_EN);\n'
        for language in range(3):
            code+=f'assert(muse_i18n_set_language({language}));\n'
            for key,*values in rows:
                code+='assert(!strcmp(muse_i18n_text(MUSE_KEY_'+key+'),'+json.dumps(values[language],ensure_ascii=False)+'));\n'
        code+='assert(!muse_i18n_set_language(99));assert(muse_i18n_language()==MUSE_LANG_ZH_TW);assert(!strcmp(muse_i18n_text("synthetic user text"),"synthetic user text"));assert(!strcmp(muse_i18n_system("Starting"),muse_i18n_text(MUSE_KEY_s005)));return 0;}'
        with tempfile.TemporaryDirectory() as d:
            p=Path(d);(p/'test.c').write_text(code)
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I',str(COMP),str(p/'test.c'),str(COMP/'muse_i18n.c'),'-o',str(p/'test')],check=True,capture_output=True,text=True)
            subprocess.run([str(p/'test')],check=True,capture_output=True,text=True)
    def test_reply_and_transcript_paths_do_not_translate_user_content(self):
        voice=(COMP/'muse_voice.c').read_text()
        self.assertIn('muse_state_set_caption("%s", text);',voice)
        ui=(COMP/'muse_ui.c').read_text()
        self.assertIn('lv_label_set_text(lbl, caption);',ui)
        history=(COMP/'muse_conversation_ui.c').read_text()
        self.assertNotIn('muse_i18n_text(text)',history)
    def test_public_language_storage_failure_and_reboot_reload(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)
            (p/'esp_err.h').write_text('typedef int esp_err_t;\n#define ESP_OK 0\n#define ESP_ERR_INVALID_ARG 2\n#define ESP_ERR_NVS_NOT_FOUND 3\n')
            (p/'nvs.h').write_text('#include <stdint.h>\n#include "esp_err.h"\ntypedef int nvs_handle_t;\n#define NVS_READONLY 0\n#define NVS_READWRITE 1\nint nvs_open(const char*,int,int*);int nvs_get_u8(int,const char*,uint8_t*);int nvs_set_u8(int,const char*,uint8_t);int nvs_commit(int);void nvs_close(int);\n')
            (p/'test.c').write_text(r'''
#include "muse_language_store.h"
#include "nvs.h"
#include <assert.h>
#include <string.h>
static int present,stored,pending,open_error,read_error,set_error,commit_error,opens,sets,commits,closes;
int nvs_open(const char *name,int mode,int *out){assert(!strcmp(name,"tamame_ui"));opens++;if(open_error)return open_error;if(!present&&mode==NVS_READONLY)return ESP_ERR_NVS_NOT_FOUND;*out=7;return 0;}
int nvs_get_u8(int h,const char*k,uint8_t*out){assert(h==7&&!strcmp(k,"language"));if(read_error)return read_error;*out=stored;return 0;}
int nvs_set_u8(int h,const char*k,uint8_t v){assert(h==7&&!strcmp(k,"language"));sets++;pending=v;return set_error;}
int nvs_commit(int h){assert(h==7);commits++;if(commit_error)return commit_error;stored=pending;present=1;return 0;}
void nvs_close(int h){assert(h==7);closes++;}
int main(void){
 assert(muse_language_load()==0&&muse_i18n_language()==MUSE_LANG_EN);assert(sets==0&&commits==0);
 for(unsigned i=0;i<3;i++){assert(muse_language_save(i)==0);muse_i18n_set_language(2);assert(muse_language_load()==0);assert(muse_i18n_language()==i);}
 int old=opens;assert(muse_language_save(99)==ESP_ERR_INVALID_ARG&&opens==old);
 stored=99;assert(muse_language_load()==0&&muse_i18n_language()==MUSE_LANG_EN);
 stored=1;open_error=41;assert(muse_language_load()==41&&muse_i18n_language()==MUSE_LANG_EN);assert(muse_language_save(2)==41);open_error=0;
 read_error=42;assert(muse_language_load()==42&&muse_i18n_language()==MUSE_LANG_EN);read_error=0;
 set_error=43;old=commits;assert(muse_language_save(2)==43&&commits==old&&stored==1);set_error=0;
 commit_error=44;assert(muse_language_save(2)==44&&stored==1);commit_error=0;assert(closes>0);
 assert(muse_language_save(2)==0);assert(muse_language_load()==0&&muse_i18n_language()==MUSE_LANG_ZH_TW);
 return 0;}
''')
            subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I',str(p),'-I',str(COMP),str(p/'test.c'),str(COMP/'muse_language_store.c'),str(COMP/'muse_i18n.c'),'-o',str(p/'test')],check=True,capture_output=True,text=True)
            subprocess.run([str(p/'test')],check=True,capture_output=True,text=True)
