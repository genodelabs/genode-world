/*
 * \brief  setcontext/getcontext/makecontext/swapcontext support library
 * \author Alexander Tormasov
 * \date   2021-03-12
 */

#ifdef __cplusplus
extern "C" {
#endif

void *alloc_secondary_stack(char const *name, unsigned long stack_size);
void free_secondary_stack(void *stack);

#ifdef __cplusplus
}
#endif
