/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* Fish Audio HTTPS streaming, isolated from the Muse connection task.
 * API: https://docs.fish.audio/api-reference/endpoint/openapi-v1/text-to-speech
 * The ring has one producer and one consumer. Only the chat task owns s_job;
 * a cancelled job is freed after the worker's final release-store. */
#include "muse_fish_tts.h"
#include "muse_speech_text.h"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <new>
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#if CONFIG_MUSE_FISH_TTS
namespace {
constexpr uint32_t RING = 64 * 1024;
constexpr size_t TEXT_LIMIT = 1024;
struct job_t {
    std::atomic<uint32_t> written{0}, read{0};
    std::atomic<bool> cancel{false}, done{false};
    uint32_t gen = 0;
    bool failed = true;  /* published by done */
    char text[TEXT_LIMIT]{};
    char url[4096]{};
    uint8_t ring[RING]{};
};
job_t *s_job;
void wipe(char *p, size_t n) { volatile char *v = p; while(n--) *v++ = 0; }
void release_job(void) {
    if (!s_job || !s_job->done.load(std::memory_order_acquire)) return;
    s_job->~job_t(); heap_caps_free(s_job); s_job = nullptr;
}
void worker(void *arg) {
    auto *j = static_cast<job_t *>(arg);
    esp_http_client_handle_t client = nullptr;
    char *body = nullptr;
    cJSON *root = cJSON_CreateObject();
    char auth[264] = {};
    bool ok = false;
    int64_t deadline = esp_timer_get_time() + 120000000LL;
    do {
        if (!j->url[0] && (!root || !cJSON_AddStringToObject(root,"text",j->text) ||
            !cJSON_AddStringToObject(root,"reference_id",CONFIG_MUSE_FISH_VOICE_ID) ||
            !cJSON_AddStringToObject(root,"language","zh-TW") ||
            !cJSON_AddStringToObject(root,"format","mp3") ||
            !cJSON_AddNumberToObject(root,"sample_rate",32000) ||
            !cJSON_AddNumberToObject(root,"mp3_bitrate",64))) break;
        if (!j->url[0]) {
            body = cJSON_PrintUnformatted(root);
            if (!body) break;
        }
        esp_http_client_config_t cfg = {};
        cfg.url = j->url[0] ? j->url : "https://api.fish.audio/v1/tts";
        cfg.timeout_ms = 15000;
        cfg.buffer_size = 2048;
        cfg.buffer_size_tx = 2048;
        cfg.crt_bundle_attach = esp_crt_bundle_attach;
        cfg.disable_auto_redirect = true;
        client = esp_http_client_init(&cfg);
        if (!client) break;
        if (j->url[0]) {
            /* Signed media URL: GET, no provider or device credentials. */
            if (j->cancel.load() || esp_http_client_set_method(client,HTTP_METHOD_GET)!=ESP_OK ||
                esp_http_client_open(client,0)!=ESP_OK) break;
        } else {
        int n = snprintf(auth,sizeof(auth),"Bearer %s",CONFIG_MUSE_FISH_API_KEY);
        if (n < 0 || static_cast<size_t>(n) >= sizeof(auth)) break;
        if (esp_http_client_set_method(client,HTTP_METHOD_POST) != ESP_OK ||
            esp_http_client_set_header(client,"Authorization",auth) != ESP_OK ||
            esp_http_client_set_header(client,"Content-Type","application/json") != ESP_OK ||
            esp_http_client_set_header(client,"model",CONFIG_MUSE_FISH_MODEL) != ESP_OK) break;
        const size_t body_len = strlen(body);
        if (j->cancel.load() || esp_http_client_open(client,body_len) != ESP_OK) break;
        size_t sent = 0;
        while (sent < body_len && !j->cancel.load()) {
            int nsend = esp_http_client_write(client,body+sent,body_len-sent);
            if (nsend <= 0) break;
            sent += static_cast<size_t>(nsend);
        }
        if (sent != body_len || j->cancel.load()) break;
        }
        if (esp_http_client_fetch_headers(client) < 0) break;
        int status=esp_http_client_get_status_code(client);
        ESP_LOGI("muse_audio","%s HTTP status %d",j->url[0]?"Attachment":"TTS",status);
        if(status!=200) break;
        size_t total = 0;
        bool read_error = false;
        uint8_t chunk[2048];
        while (!j->cancel.load() && esp_timer_get_time() < deadline) {
            int count = esp_http_client_read(client,reinterpret_cast<char *>(chunk),sizeof(chunk));
            if (count < 0) { read_error = true; break; }
            if (!count) {
                if (!esp_http_client_is_complete_data_received(client)) read_error = true;
                break;
            }
            total += static_cast<size_t>(count);
            if (total > 2 * 1024 * 1024) { read_error = true; break; }
            size_t off = 0;
            while (off < static_cast<size_t>(count) && !j->cancel.load() && esp_timer_get_time() < deadline) {
                uint32_t w = j->written.load(std::memory_order_relaxed);
                uint32_t used = w - j->read.load(std::memory_order_acquire);
                size_t ncopy = RING-used;
                if (ncopy > static_cast<size_t>(count)-off) ncopy = count-off;
                if (ncopy > RING-(w%RING)) ncopy = RING-(w%RING);
                if (!ncopy) {vTaskDelay(pdMS_TO_TICKS(10));continue;}
                memcpy(j->ring+(w%RING),chunk+off,ncopy);
                j->written.store(w+ncopy,std::memory_order_release);off+=ncopy;
            }
        }
        ok = !read_error && !j->cancel.load() && esp_timer_get_time()<deadline && total>0;
    } while(false);
    if (client) {
        esp_http_client_set_header(client,"Authorization","");
        esp_http_client_close(client);esp_http_client_cleanup(client);
    }
    wipe(auth,sizeof(auth));
    if (body) {wipe(body,strlen(body));cJSON_free(body);}
    cJSON_Delete(root);wipe(j->text,sizeof(j->text));wipe(j->url,sizeof(j->url));
    j->failed = !ok;
    j->done.store(true,std::memory_order_release);
    vTaskDeleteWithCaps(nullptr); /* no access to j after publishing done */
}
}
#endif

bool muse_fish_tts_configured(void) {
#if CONFIG_MUSE_FISH_TTS
    const char *key = CONFIG_MUSE_FISH_API_KEY, *voice = CONFIG_MUSE_FISH_VOICE_ID;
    if (!*key || !*voice || strlen(key)>256 || strlen(voice)>128 || strpbrk(key,"\r\n")) return false;
    for (const unsigned char *c=reinterpret_cast<const unsigned char *>(key);*c;c++)
        if (*c<33 || *c>126) return false;
    for (const char *c=voice;*c;c++)
        if (!((*c>='a'&&*c<='z')||(*c>='A'&&*c<='Z')||(*c>='0'&&*c<='9')||*c=='_'||*c=='-')) return false;
    return true;
#else
    return false;
#endif
}
int muse_fish_tts_start(const char *text,uint32_t generation) {
#if CONFIG_MUSE_FISH_TTS
    muse_fish_tts_reap();
    if (s_job) return 0;
    if (!muse_fish_tts_configured() || !text || !*text || strlen(text)>=TEXT_LIMIT) return -1;
    std::string speech = muse_speech::clean(text);
    if (speech.empty()) return 2; /* captions remain, no network request */
    if (speech.size() >= TEXT_LIMIT) return -1;
    void *mem = heap_caps_malloc(sizeof(job_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if (!mem) return -1;
    s_job = new(mem) job_t;
    s_job->gen=generation;memcpy(s_job->text,speech.c_str(),speech.size()+1);
    if (xTaskCreatePinnedToCoreWithCaps(worker,"fish_tts",16*1024,s_job,4,nullptr,0,
                                      MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT) != pdPASS) {
        wipe(s_job->text,sizeof(s_job->text));s_job->~job_t();heap_caps_free(s_job);s_job=nullptr;return -1;
    }
    return 1;
#else
    (void)text;(void)generation;return -1;
#endif
}
bool muse_audio_url_valid(const char *url) {
    if (!url || strncmp(url,"https://",8) || strlen(url)>=4096) return false;
    const char *host=url+8, *end=strchr(host,'/');
    if (!end || end==host) return false;
    for (const unsigned char *c=reinterpret_cast<const unsigned char *>(url);*c;++c)
        if (*c<=32 || *c>=127 || *c=='\\' || *c=='#') return false;
    for(const char *c=host;c<end;++c) if(*c=='@')return false;
    return true;
}
int muse_audio_url_start(const char *url,uint32_t generation) {
#if CONFIG_MUSE_FISH_TTS
    muse_fish_tts_reap();
    if (s_job) return 0;
    if (!muse_audio_url_valid(url)) return -1;
    void *mem=heap_caps_malloc(sizeof(job_t),MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    if (!mem) return -1;
    s_job=new(mem) job_t;s_job->gen=generation;
    memcpy(s_job->url,url,strlen(url)+1);
    if(xTaskCreatePinnedToCoreWithCaps(worker,"muse_media",16*1024,s_job,4,nullptr,0,
                                     MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT)!=pdPASS) {
        wipe(s_job->url,sizeof(s_job->url));s_job->~job_t();heap_caps_free(s_job);s_job=nullptr;return -1;
    }
    return 1;
#else
    (void)url;(void)generation;return -1;
#endif
}
size_t muse_fish_tts_read(uint8_t *dst,size_t cap,uint32_t generation,bool *ended,bool *failed) {
    *ended=false;*failed=false;
#if CONFIG_MUSE_FISH_TTS
    if (!s_job || s_job->gen!=generation || s_job->cancel.load()) return 0;
    uint32_t r=s_job->read.load(std::memory_order_relaxed);
    size_t n=s_job->written.load(std::memory_order_acquire)-r;
    if(n>cap)n=cap;
    if(n>RING-r%RING)n=RING-r%RING;
    if(n) {memcpy(dst,s_job->ring+r%RING,n);s_job->read.store(r+n,std::memory_order_release);}
    if(s_job->done.load(std::memory_order_acquire) &&
       s_job->written.load(std::memory_order_acquire)==r+n) {
        *ended=true;*failed=s_job->failed;release_job();
    }
    return n;
#else
    (void)dst;(void)cap;(void)generation;return 0;
#endif
}
void muse_fish_tts_cancel(void) {
#if CONFIG_MUSE_FISH_TTS
    if(s_job)s_job->cancel.store(true);
#endif
}
void muse_fish_tts_reap(void) {
#if CONFIG_MUSE_FISH_TTS
    if(s_job&&s_job->cancel.load())release_job();
#endif
}
