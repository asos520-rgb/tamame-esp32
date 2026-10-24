/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#include "sdkconfig.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include "muse_recent.h"
#include <stdlib.h>
#include <string.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static SemaphoreHandle_t s_mutex;
#define LOCK() xSemaphoreTake(s_mutex,portMAX_DELAY)
#define UNLOCK() xSemaphoreGive(s_mutex)
#ifndef RECENT_ALLOC
#define RECENT_ALLOC(n) heap_caps_malloc(n,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT)
#endif
#else
#include <pthread.h>
static pthread_mutex_t s_mutex=PTHREAD_MUTEX_INITIALIZER;
#define LOCK() pthread_mutex_lock(&s_mutex)
#define UNLOCK() pthread_mutex_unlock(&s_mutex)
#ifndef RECENT_ALLOC
#define RECENT_ALLOC(n) malloc(n)
#endif
#endif
static bool s_started,s_failed;
typedef struct {muse_recent_meta_t meta;char *text;} entry_t;
typedef struct {char id[80];char *text;size_t len,cap;bool failed;} draft_t;
static entry_t s_entries[MUSE_RECENT_COUNT];
static draft_t s_drafts[MUSE_RECENT_COUNT];
static unsigned s_count,s_next;
static uint32_t s_serial;
void muse_recent_start(void) {
    if(s_started)return;
#ifdef ESP_PLATFORM
    s_mutex=xSemaphoreCreateMutex();if(!s_mutex)return;
#endif
    s_started=true;
}
void muse_recent_begin(void) {
    /* Drafts are network-task-owned. Completed entries are protected separately. */
    for(unsigned i=0;i<MUSE_RECENT_COUNT;i++){free(s_drafts[i].text);s_drafts[i]=(draft_t){0};}
}
static void failed(void){LOCK();s_failed=true;UNLOCK();}
static draft_t *draft(const char *id) {
    for(unsigned i=0;i<MUSE_RECENT_COUNT;i++)if(!strcmp(s_drafts[i].id,id))return &s_drafts[i];
    for(unsigned i=0;i<MUSE_RECENT_COUNT;i++)if(!s_drafts[i].id[0]){strlcpy(s_drafts[i].id,id,sizeof s_drafts[i].id);return &s_drafts[i];}
    return NULL;
}
void muse_recent_append(const char *id,const char *text) {
    if(!s_started||!id||!text||!*text)return;
    draft_t *d=draft(id);if(!d||d->failed)return;
    size_t n=strlen(text);
    if(n>MUSE_RECENT_TEXT_MAX-d->len){d->failed=true;failed();return;}
    size_t need=d->len+n+1;
    if(need>d->cap) {
        size_t cap=d->cap?d->cap:256;while(cap<need)cap*=2;if(cap>MUSE_RECENT_TEXT_MAX+1)cap=MUSE_RECENT_TEXT_MAX+1;
        char *p=RECENT_ALLOC(cap);if(!p){d->failed=true;failed();return;}
        if(d->len){memcpy(p,d->text,d->len);}
        free(d->text);d->text=p;d->cap=cap;
    }
    memcpy(d->text+d->len,text,n+1);d->len+=n;
}
void muse_recent_complete(const char *id,const char *session,const char *final_text) {
    if(!s_started||!id||!session)return;
    draft_t *d=NULL;for(unsigned i=0;i<MUSE_RECENT_COUNT;i++)if(!strcmp(s_drafts[i].id,id)){d=&s_drafts[i];break;}
    const char *text=final_text?final_text:d&&!d->failed&&d->text?d->text:"";
    if(!final_text&&d&&d->failed){failed();return;}
    size_t n=strlen(text);if(n>MUSE_RECENT_TEXT_MAX){failed();return;}
    char *copy=RECENT_ALLOC(n+1);if(!copy){failed();return;}memcpy(copy,text,n+1);
    LOCK();
    for(unsigned i=0;i<MUSE_RECENT_COUNT;i++) {
        if(s_entries[i].text&&!strcmp(s_entries[i].meta.message_id,id)&&!strcmp(s_entries[i].meta.session_id,session)) {
            free(s_entries[i].text);s_entries[i].text=copy;s_entries[i].meta.bytes=n;
            UNLOCK();if(d){free(d->text);*d=(draft_t){0};}return;
        }
    }
    entry_t *e=&s_entries[s_next];free(e->text);*e=(entry_t){.text=copy};
    e->meta.serial=++s_serial;e->meta.bytes=n;e->meta.dedicated=session[0]!=0;
    strlcpy(e->meta.message_id,id,sizeof e->meta.message_id);strlcpy(e->meta.session_id,session,sizeof e->meta.session_id);
    s_next=(s_next+1)%MUSE_RECENT_COUNT;if(s_count<MUSE_RECENT_COUNT)s_count++;
    UNLOCK();if(d){free(d->text);*d=(draft_t){0};}
}
unsigned muse_recent_count(void){if(!s_started)return 0;LOCK();unsigned n=s_count;UNLOCK();return n;}
bool muse_recent_failed(void){if(!s_started)return false;LOCK();bool result=s_failed;UNLOCK();return result;}
bool muse_recent_summary(unsigned index,muse_recent_meta_t *meta,char *preview,size_t capacity) {
    if(!s_started||!capacity)return false;
    LOCK();
    if(index>=s_count){UNLOCK();return false;}
    entry_t *e=&s_entries[(s_next+MUSE_RECENT_COUNT-1-index)%MUSE_RECENT_COUNT];*meta=e->meta;
    size_t n=e->meta.bytes<capacity-1?e->meta.bytes:capacity-1;
    while(n<e->meta.bytes&&n>0&&((unsigned char)e->text[n]&0xc0)==0x80)n--;
    memcpy(preview,e->text,n);preview[n]=0;UNLOCK();return true;
}
char *muse_recent_copy(uint32_t serial,muse_recent_meta_t *meta) {
    if(!s_started)return NULL;
    LOCK();char *copy=NULL;
    for(unsigned i=0;i<MUSE_RECENT_COUNT;i++)if(s_entries[i].text&&s_entries[i].meta.serial==serial) {
        *meta=s_entries[i].meta;copy=RECENT_ALLOC(meta->bytes+1);
        if(copy){memcpy(copy,s_entries[i].text,meta->bytes+1);}
        break;
    }
    UNLOCK();return copy;
}
#endif
