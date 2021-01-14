#ifndef TEMPLATE_LIB_COMMON_H
#define TEMPLATE_LIB_COMMON_H


/* todo: can drop to c89 by axing this */
#include <assert.h>

#include "bits.h"
#include "tlstatus.h"
#include "allocator.h"

/**
 * Extra indirection gets the defines to paste in properly so we expand to the macro
 * values instead of the macro names
 */
#define _TLCONCAT(l,r) l##r
#define TLCONCAT(l,r) _TLCONCAT(l,r)
#define TLSYMBOL(prefix,name) TLCONCAT(prefix,TLCONCAT(_,name))

#ifdef NDEBUG
#define TL_INIT_VAL 0x00
#else
#define TL_INIT_VAL 0x45
#endif


#endif //TEMPLATE_LIB_COMMON_H
