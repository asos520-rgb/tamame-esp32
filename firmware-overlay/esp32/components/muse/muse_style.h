/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
typedef struct { uint32_t version; uint8_t background,clothing,accessory,sunglasses; } muse_style_t;
static inline bool muse_style_valid(const muse_style_t *s) {return s&&s->version==2&&s->background<=5&&s->clothing<3&&s->accessory<3&&s->sunglasses<2;}
void muse_style_start(void);
bool muse_style_snapshot(muse_style_t *out);
bool muse_style_save(const muse_style_t *style);
/* 0 waiting, 1 saved, -1 failed. No flash writes on the UI task. */
int muse_style_feedback(void);
