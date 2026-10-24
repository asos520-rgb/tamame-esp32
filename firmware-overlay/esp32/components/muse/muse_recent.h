/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
#define MUSE_RECENT_COUNT 8
#define MUSE_RECENT_TEXT_MAX (256u*1024u)
typedef struct {uint32_t serial;size_t bytes;bool dedicated;char message_id[80],session_id[48];} muse_recent_meta_t;
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
void muse_recent_start(void);
void muse_recent_begin(void);
void muse_recent_append(const char *id,const char *text);
void muse_recent_complete(const char *id,const char *session,const char *final_text);
unsigned muse_recent_count(void);
bool muse_recent_summary(unsigned index,muse_recent_meta_t *meta,char *preview,size_t capacity);
/* Caller frees this snapshot. IDs, not row positions, keep clicks stable. */
char *muse_recent_copy(uint32_t serial,muse_recent_meta_t *meta);
bool muse_recent_failed(void);
#else
static inline void muse_recent_start(void){}
static inline void muse_recent_begin(void){}
static inline void muse_recent_append(const char *id,const char *text){(void)id;(void)text;}
static inline void muse_recent_complete(const char *id,const char *session,const char *text){(void)id;(void)session;(void)text;}
#endif
#ifdef __cplusplus
}
#endif
