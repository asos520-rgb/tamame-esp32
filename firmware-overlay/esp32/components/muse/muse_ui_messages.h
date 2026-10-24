/* SPDX-License-Identifier: Apache-2.0 */
/* Compatibility API for fixed system errors; catalog is muse_i18n_catalog.def. */
#pragma once
#include "muse_i18n.h"
static inline const char *muse_ui_message(const char *fixed_system_message){return muse_i18n_system(fixed_system_message);}
