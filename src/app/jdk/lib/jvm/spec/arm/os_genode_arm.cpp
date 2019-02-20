#include <base/log.h>

extern "C" {
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <pthread_np.h>
}

#include <runtime/frame.inline.hpp>
#include <utilities/growableArray.hpp>

#include <os_bsd.hpp>

#if 0
#define WARN_NOT_IMPL Genode::warning(__func__, " not implemented (jvm_arm) from ", __builtin_return_address(0));
#else
#define WARN_NOT_IMPL
#endif

size_t os::Posix::_vm_internal_thread_min_stack_allowed = 64 * K;
size_t os::Posix::_compiler_thread_min_stack_allowed = 64 * K;
size_t os::Posix::_java_thread_min_stack_allowed = 64 * K;


/**********************
 ** Atomic functions **
 **********************/

typedef jlong load_long_func_t(volatile jlong*);
load_long_func_t* os::atomic_load_long_func = os::atomic_load_long_bootstrap;

typedef void store_long_func_t(jlong, volatile jlong*);
store_long_func_t* os::atomic_store_long_func = os::atomic_store_long_bootstrap;

typedef jint cmpxchg_func_t(jint, jint, volatile jint*);
cmpxchg_func_t* os::atomic_cmpxchg_func = os::atomic_cmpxchg_bootstrap;

typedef jint  atomic_xchg_func_t(jint exchange_value, volatile jint *dest);
atomic_xchg_func_t *os::atomic_xchg_func = os::atomic_xchg_bootstrap;

typedef jlong cmpxchg_long_func_t(jlong, jlong, volatile jlong*);
cmpxchg_long_func_t* os::atomic_cmpxchg_long_func = os::atomic_cmpxchg_long_bootstrap;

typedef jint  atomic_add_func_t(jint add_value, volatile jint *dest);
atomic_add_func_t * os::atomic_add_func = os::atomic_add_bootstrap;


jlong os::atomic_load_long_bootstrap(volatile jlong *src)
{
	return __atomic_load_n(src, __ATOMIC_ACQUIRE);
}


void os::atomic_store_long_bootstrap(jlong val, volatile jlong *dest)
{
	__atomic_store_n(dest, val, __ATOMIC_RELEASE);
}


jint os::atomic_xchg_bootstrap(jint exchange_value, volatile jint *dest)
{
	return __atomic_exchange_n(dest, exchange_value, __ATOMIC_ACQUIRE);
}


jint os::atomic_cmpxchg_bootstrap(jint compare_value,
                                  jint exchange_value,
                                  volatile jint *dest)
{
	__atomic_compare_exchange_n(dest, &compare_value,
	                            exchange_value, false,
	                            __ATOMIC_ACQUIRE,
	                            __ATOMIC_ACQUIRE);
	return compare_value;
}


jlong os::atomic_cmpxchg_long_bootstrap(jlong compare_value,
                                        jlong exchange_value,
                                        volatile jlong *dest)
{
	__atomic_compare_exchange_n(dest, &compare_value,
	                            exchange_value, false,
	                            __ATOMIC_ACQUIRE,
	                            __ATOMIC_ACQUIRE);
	return compare_value;
}


jint os::atomic_add_bootstrap(jint add_value, volatile jint *dest)
{
	return __atomic_add_fetch(dest, add_value, __ATOMIC_ACQUIRE);
}


void os::print_context(outputStream* st, const void* context)
{
	WARN_NOT_IMPL;
}


void os::print_register_info(outputStream *st, const void *context)
{
	WARN_NOT_IMPL;
}


void os::initialize_thread(Thread* thr)
{
	WARN_NOT_IMPL;
}


void os::Bsd::init_thread_fpu_state()
{
	WARN_NOT_IMPL;
}


void os::setup_fpu()
{
	WARN_NOT_IMPL;
}


char* os::non_memory_address_word()
{
	WARN_NOT_IMPL;
	return (char*)-1;
}


bool os::is_allocatable(size_t bytes)
{
	return true;
}


int os::extra_bang_size_in_bytes()
{
	WARN_NOT_IMPL;
	return 0;
}


frame os::current_frame()
{
	WARN_NOT_IMPL;
	return frame();
}


frame os::get_sender_for_C_frame(frame* fr)
{
	WARN_NOT_IMPL;
	return frame();
}


frame os::fetch_frame_from_context(const void* ucVoid)
{
	WARN_NOT_IMPL;
	return frame();
}


void os::verify_stack_alignment()
{
}


address os::current_stack_pointer()
{
	address sp = 0;
	asm volatile ("mov %0, sp\n" : "=r" (sp) : :);
	return sp;
}


size_t os::current_stack_size()
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_get_np(pthread_self(), &attr);

	size_t size; void *addr;
	pthread_attr_getstack(&attr, &addr, &size);

	pthread_attr_destroy(&attr);

	return size;
}


address os::current_stack_base()
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_get_np(pthread_self(), &attr);

	size_t size; void *addr;
	pthread_attr_getstack(&attr, &addr, &size);

	pthread_attr_destroy(&attr);
	return (address)addr + size;
}


/**
 * POSIX
 */

size_t os::Posix::default_stack_size(os::ThreadType thr_type)
{
	WARN_NOT_IMPL;
	return 64*1024;
}


/**
 * BSD
 */

void os::Bsd::ucontext_set_pc(ucontext_t * uc, address pc)
{
	WARN_NOT_IMPL;
}


address os::Bsd::ucontext_get_pc(const ucontext_t* uc)
{
	WARN_NOT_IMPL;
	return 0;
}


void VM_Version::get_os_cpu_info()
{
	static bool done = false;
	if (done) return;

	_arm_arch = 7;
}

void VM_Version::early_initialize()
{
	get_os_cpu_info();
}


extern "C" JNIEXPORT int
JVM_handle_bsd_signal(int sig,
                      siginfo_t* info,
                      void* ucVoid,
                      int abort_if_unrecognized)
{
	WARN_NOT_IMPL;
	return -1;
}


/**
 * JavaThread
 */
bool JavaThread::pd_get_top_frame_for_signal_handler(frame* fr_addr,
                                                      void* ucontext, bool isInJava)
{
	WARN_NOT_IMPL;
	return false;
}


void JavaThread::cache_global_variables()
{
	WARN_NOT_IMPL;
}


