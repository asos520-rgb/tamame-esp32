/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { MUSE_LANG_EN=0, MUSE_LANG_ZH_CN=1, MUSE_LANG_ZH_TW=2, MUSE_LANG_COUNT=3 } muse_language_t;
#include "muse_i18n_catalog.def"
muse_language_t muse_i18n_language(void);
bool muse_i18n_set_language(unsigned language);
const char *muse_i18n_language_name(unsigned language);
/* Fixed catalog keys only; never pass user content here. */
const char *muse_i18n_text(const char *key);
const char *muse_i18n_system(const char *message);
#ifdef __cplusplus
}
#endif
