/* SPDX-License-Identifier: Apache-2.0 */
#include "muse_i18n.h"
#include <stdatomic.h>
#include <stddef.h>
#include <string.h>
static atomic_uint language=MUSE_LANG_EN;
typedef struct { const char *text[MUSE_LANG_COUNT]; } entry_t;
static const entry_t entries[]={
#define MUSE_STRING(id,en,cn,tw) {{en,cn,tw}},
#include "muse_i18n_catalog.def"
#undef MUSE_STRING
};
enum {
#define MUSE_STRING(id,en,cn,tw) ID_##id,
#include "muse_i18n_catalog.def"
#undef MUSE_STRING
};
static const struct {const char *source;unsigned id;} aliases[]={
#define MUSE_ALIAS(source,id) {source,ID_##id},
#include "muse_i18n_catalog.def"
#undef MUSE_ALIAS
};
muse_language_t muse_i18n_language(void){return (muse_language_t)atomic_load(&language);}
bool muse_i18n_set_language(unsigned value){if(value>=MUSE_LANG_COUNT)return false;atomic_store(&language,value);return true;}
const char *muse_i18n_language_name(unsigned value){static const char *const names[]={"English","简体中文","繁體中文"};return value<MUSE_LANG_COUNT?names[value]:names[0];}
const char *muse_i18n_text(const char *key){
 if(!key)return "";
 unsigned lang=atomic_load(&language);
 for(unsigned i=0;i<sizeof(entries)/sizeof(entries[0]);i++)if(!strcmp(key,entries[i].text[MUSE_LANG_ZH_TW]))return entries[i].text[lang];
 return key;
}
const char *muse_i18n_system(const char *message){
 if(!message)return "";
 unsigned lang=atomic_load(&language);
 for(unsigned i=0;i<sizeof(aliases)/sizeof(aliases[0]);i++)if(!strcmp(message,aliases[i].source))return entries[aliases[i].id].text[lang];
 if(!strncmp(message,"Can't reach ",12))return muse_i18n_text(MUSE_KEY_s147);
 if(!strncmp(message,"VM connect failed (HTTP ",24))return muse_i18n_text(MUSE_KEY_s127);
 return message;
}
