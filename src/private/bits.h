#ifndef TEMPLATE_LIB_BITS_H
#define TEMPLATE_LIB_BITS_H

#include <limits.h>

#ifndef SIZE_MAX
#error "size_t (some unsigned integer) and SIZE_MAX (max value of size_t) must be defined. Did you forget to include stdint.h?"
#endif

#ifndef CHAR_BIT
#error "CHAR_BIT is not defined. Are you on a standard C89 or higher C compiler?"
#endif

#if (CHAR_BIT != 8)
#error "This library is built for CHAR_BIT defined as 8. Sorry about that!"
#endif

#if (SIZE_MAX == 0xFFFFFFFF)
#define TL_SIZE_T_BYTES 8			/* Maybe 32-bit compiler */
#elif (SIZE_MAX == 0xFFFFFFFFFFFFFFFF)
#define TL_SIZE_T_BYTES 16			/* Maybe 64-bit compiler */
#else
#error "Template_lib currently only supports 32-bit and 64-bit size_t. Sorry about that!"
#endif


#endif //TEMPLATE_LIB_BITS_H
