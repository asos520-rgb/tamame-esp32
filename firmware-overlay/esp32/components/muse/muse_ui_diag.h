/* Public-edition staging: local UI additions and/or privacy review; see source manifest. */
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Temporary freeze diagnostics: observes progress, never resets or changes UI. */
#pragma once
#include "sdkconfig.h"
#if CONFIG_MUSE_BOARD_WAVESHARE_S3_175C
#include "lvgl.h"
void muse_ui_diag_start(lv_display_t *display);
void muse_ui_diag_frame(void);
void muse_ui_diag_transfer_done_from_isr(void);
#else
#define muse_ui_diag_start(display) ((void)0)
#define muse_ui_diag_frame() ((void)0)
#define muse_ui_diag_transfer_done_from_isr() ((void)0)
#endif
