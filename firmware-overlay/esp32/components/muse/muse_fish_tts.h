/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* Fish Audio MP3 producer. Called only by the chat task; HTTPS runs separately. */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
bool muse_fish_tts_configured(void);
/* 2 empty speech (silent captions), 1 started, 0 previous cancelled request still stopping, -1 allocation/error. */
int muse_fish_tts_start(const char *text, uint32_t generation);
/* HTTPS media GET shares the bounded producer; sends no authorization header. */
bool muse_audio_url_valid(const char *url);
int muse_audio_url_start(const char *url, uint32_t generation);
size_t muse_fish_tts_read(uint8_t *dst, size_t cap, uint32_t generation,
                          bool *ended, bool *failed);
void muse_fish_tts_cancel(void);
void muse_fish_tts_reap(void);
#ifdef __cplusplus
}
#endif
