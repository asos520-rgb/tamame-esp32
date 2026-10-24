#pragma once
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <thread>
#include <vector>
#define ESP_OK 0
#define ESP_LOGI(...) ((void)0)
#define HTTP_METHOD_POST 1
#define HTTP_METHOD_GET 0
#define MALLOC_CAP_SPIRAM 1
#define MALLOC_CAP_8BIT 2
#define pdPASS 1
#define pdMS_TO_TICKS(x) (x)
inline std::vector<std::thread> workers;
inline std::vector<uint8_t> response;
inline std::string request_body,request_url;
inline int request_method=-1;
inline std::map<std::string,std::string> request_headers;
inline int http_status=200;
inline bool task_fail=false,redirect_disabled=false,cert_verified=false;
inline void *heap_caps_malloc(size_t n,int) {return malloc(n);}
inline void heap_caps_free(void *p) {free(p);}
inline int64_t esp_timer_get_time() {
 return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
inline void vTaskDelay(int) {std::this_thread::sleep_for(std::chrono::milliseconds(1));}
inline void vTaskDeleteWithCaps(void *) {}
inline int xTaskCreatePinnedToCoreWithCaps(void (*fn)(void*),const char*,int,void *arg,int,void*,int,int) {
 if(task_fail)return 0;
 workers.emplace_back(fn,arg);return pdPASS;
}
inline int esp_crt_bundle_attach(void*) {return 0;}
struct esp_http_client_config_t {
 const char *url=nullptr;int timeout_ms=0,buffer_size=0,buffer_size_tx=0;
 int (*crt_bundle_attach)(void*)=nullptr;bool disable_auto_redirect=false;
};
struct fake_http {size_t off=0;};
using esp_http_client_handle_t=fake_http*;
inline fake_http *esp_http_client_init(const esp_http_client_config_t *c) {
 request_url=c->url;
 redirect_disabled=c->disable_auto_redirect;cert_verified=c->crt_bundle_attach!=nullptr;
 request_body.clear();request_headers.clear();return new fake_http;
}
inline int esp_http_client_set_method(fake_http*,int m) {request_method=m;return ESP_OK;}
inline int esp_http_client_set_header(fake_http*,const char *k,const char *v) {
 if(*v)request_headers[k]=v;return ESP_OK;
}
inline int esp_http_client_open(fake_http*,int) {return ESP_OK;}
inline int esp_http_client_write(fake_http*,const char *p,int n) {
 int out=std::min(n,7);request_body.append(p,out);return out;
}
inline int64_t esp_http_client_fetch_headers(fake_http*) {return response.size();}
inline int esp_http_client_get_status_code(fake_http*) {return http_status;}
inline int esp_http_client_read(fake_http *h,char *p,int n) {
 size_t count=std::min(static_cast<size_t>(n),response.size()-h->off);
 if(count)memcpy(p,response.data()+h->off,count);h->off+=count;return count;
}
inline bool esp_http_client_is_complete_data_received(fake_http *h) {return h->off==response.size();}
inline int esp_http_client_close(fake_http*) {return ESP_OK;}
inline int esp_http_client_cleanup(fake_http *h) {delete h;return ESP_OK;}
