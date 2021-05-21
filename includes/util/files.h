#ifndef _FILES_H
#define _FILES_H

#include "util/util.h"

// ------- FLAGS ------- //
#define O_NOFLAG 0
#define O_CREATE 1
#define O_LOCK   2

#define IS_FLAG_SET(val, flag) \
    (val >> flag) & 1U

#define CLEAR_FLAG(val, flag) \
    val &= ~(1U << flag)

#endif