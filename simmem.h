/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#ifdef _MSC_VER
#include <malloc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define guarded_malloc	malloc
#define guarded_realloc	realloc
#define guarded_free	free
#else
void * guarded_malloc(int size);
void *guarded_realloc(void *old, int newsize);
void guarded_free(void *p);
#endif

#ifdef __cplusplus
}
#endif
