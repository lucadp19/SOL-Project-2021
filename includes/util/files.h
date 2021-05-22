#ifndef _FILES_H
#define _FILES_H

#include "util/util.h"

// ------- FLAGS ------- //
#define O_NOFLAG 0
#define O_CREATE 1
#define O_LOCK   2

/**
 * Checks is flag (O_CREATE or O_LOCK) is set in val.
 */
#define IS_FLAG_SET(val, flag) \
    (val >> (flag/2)) & 1U // only works because flag = 1 or 2

#define CLEAR_FLAG(val, flag) \
    val &= ~(1U << (flag/2))

#endif