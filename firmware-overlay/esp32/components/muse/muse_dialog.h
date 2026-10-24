/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
#pragma once
#include <stdint.h>
#define MUSE_DIALOG_PAD_X 12
#define MUSE_DIALOG_PAD_Y 10
#define MUSE_DIALOG_RECT_COUNT 12
#define MUSE_DIALOG_OUTER 0xB58AF5u
#define MUSE_DIALOG_GAP   0x35204Fu
#define MUSE_DIALOG_INNER 0x7650A8u
#define MUSE_DIALOG_FILL  0x1B102Du
typedef struct {int x,y,w,h;uint32_t rgb;} muse_dialog_rect_t;
/* Four nested stepped silhouettes, three opaque rectangles per silhouette.
 * No textures, masks, framebuffer or per-frame allocation. */
int muse_dialog_rects(int w,int h,muse_dialog_rect_t *out,int capacity);
int muse_dialog_height(int measured_text_height,int reserved_text_height);
