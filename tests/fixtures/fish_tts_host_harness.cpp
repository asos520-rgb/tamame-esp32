#include <cassert>
#include <cstdio>
#include "fish_tts_host_platform.h"
#include "muse_fish_tts.cpp"
static void join_workers() {for(auto &t:workers)t.join();workers.clear();}
static std::vector<uint8_t> drain(uint32_t gen,bool *failed) {
 std::vector<uint8_t> out;uint8_t chunk[1301];bool ended=false;
 int64_t deadline=esp_timer_get_time()+10000000;
 while(!ended&&esp_timer_get_time()<deadline) {
  size_t n=muse_fish_tts_read(chunk,sizeof(chunk),gen,&ended,failed);
  out.insert(out.end(),chunk,chunk+n);if(!n)vTaskDelay(1);
 }
 assert(ended);join_workers();return out;
}
int main() {
 assert(muse_fish_tts_configured());
 assert(muse_fish_tts_start("👌",1)==2 && !s_job && workers.empty());
 response.resize(350001);for(size_t i=0;i<response.size();i++)response[i]=(i*17)%251;
 assert(muse_fish_tts_start("**你好**，台灣！🎧\"quote\"",10)==1);
 uint8_t byte;bool ended,failed;
 assert(muse_fish_tts_read(&byte,1,9,&ended,&failed)==0&&!ended);
 auto out=drain(10,&failed);assert(!failed&&out==response);
 assert(redirect_disabled&&cert_verified);
 assert(request_headers["Authorization"]=="Bearer test_api_key");
 assert(request_headers["model"]=="s2.1-pro-free");
 cJSON *body=cJSON_Parse(request_body.c_str());assert(body);
 assert(!strcmp(cJSON_GetObjectItem(body,"format")->valuestring,"mp3"));
 assert(!strcmp(cJSON_GetObjectItem(body,"language")->valuestring,"zh-TW"));
 assert(!strcmp(cJSON_GetObjectItem(body,"text")->valuestring,"你好，台灣！\"quote\""));
 assert(!strstr(request_body.c_str(),"test_api_key"));cJSON_Delete(body);
 // Signed media uses GET and never sends provider credentials or text.
 assert(!muse_audio_url_valid("http://example.com/audio.mp3"));
 assert(!muse_audio_url_valid("https://user:@example.com/a"));
 assert(!muse_audio_url_valid("https://example.com/a\r\nInjected: x"));
 assert(!muse_audio_url_valid("sandbox://a.mp3"));
 assert(muse_audio_url_start("https://media.example.com/a.mp3?sig=test",20)==1);
 assert(drain(20,&failed)==response&&!failed);
 assert(request_method==HTTP_METHOD_GET && request_headers.empty() && request_body.empty());
 assert(request_url=="https://media.example.com/a.mp3?sig=test");
 http_status=403;assert(muse_audio_url_start("https://media.example.com/expired",21)==1);
 assert(drain(21,&failed).empty()&&failed);http_status=200;
 // Rejection returns no server error bytes as audio.
 http_status=401;assert(muse_fish_tts_start("text",11)==1);
 assert(drain(11,&failed).empty()&&failed);http_status=200;
 // Cancel while the bounded ring is full, then accept the next turn.
 assert(muse_fish_tts_start("cancel",12)==1);
 std::this_thread::sleep_for(std::chrono::milliseconds(20));muse_fish_tts_cancel();
 join_workers();muse_fish_tts_reap();assert(!s_job);
 response.assign(123,0x55);assert(muse_fish_tts_start("next",13)==1);
 assert(drain(13,&failed)==response&&!failed);
 task_fail=true;assert(muse_fish_tts_start("text",14)==-1&&!s_job);task_fail=false;
 assert(muse_fish_tts_start("",15)==-1);
 std::string large(1024,'a');assert(muse_fish_tts_start(large.c_str(),15)==-1);
 puts("PASS: streamed bytes/backpressure, stale generations, cancellation, HTTP errors, partial writes, allocation failure, JSON and TLS policy");
}
