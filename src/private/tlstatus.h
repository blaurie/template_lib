#ifndef TEMPLATE_LIB_TLSTATUS_H
#define TEMPLATE_LIB_TLSTATUS_H

enum tl_status {
	TL_ERROR = 0,		/* general error, probably not used much */
	TLOK,

	TL_ENF,			/* Element not found */
	TL_OOB,			/* Out of bounds */
	TL_EAE,			/* Element already exists */

	TL_ERR_MEM,		/* Memory allocation failed */
	TL_ERRIN_MMR,		/* Malloc required in allocator struct */
	TL_ERRIN_MCR,		/* Calloc required in allocator struct */
	TL_ERRIN_MRR,		/* Realloc required in allocator struct */
	TL_ERRIN_MFR,		/* Free required in allocator struct */
	TL_ERRIN_MMSR,		/* Memset required in allocator struct */
	TL_ERRIN_MMMR,		/* Memmove required in allocator struct */

};

#endif //TEMPLATE_LIB_TLSTATUS_H
