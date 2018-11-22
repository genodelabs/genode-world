#ifndef _INCLUDE__ALLOCA_H_
#define _INCLUDE__ALLOCA_H_


static inline void *alloca(unsigned long size)
{
	return __builtin_alloca(size);
}

#endif /* _INCLUDE__ALLOCA_H_ */
