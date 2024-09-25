#include <stdatomic.h>

// Shortcuts for load-relaxed and load-acquire
#define atomic_load_relaxed(p) atomic_load_explicit((p), memory_order_relaxed)
#define atomic_load_acquire(p) atomic_load_explicit((p), memory_order_acquire)

// Atomic load-consume.
//
// Perform a load-consume, with the semantics it should have rather than the
// semantics it is defined to have in the standard. On most CPUs, this is just
// a relaxed atomic load (assuming that volatile has the new semantics specified
// in C18, as it does in virtually every C implementation ever).
#if !defined(atomic_load_consume)
#define atomic_load_consume(p) atomic_load_explicit(p, memory_order_relaxed)
#endif

// Shortcuts for store-relaxed and store-release
#define atomic_store_relaxed(p, v)                                             \
	atomic_store_explicit((p), (v), memory_order_relaxed)
#define atomic_store_release(p, v)                                             \
	atomic_store_explicit((p), (v), memory_order_release)

// Device memory fences
//
// The atomic_thread_fence() builtin only generates a fence for CPU threads,
// which means the compiler is allowed to use a DMB ISH instruction. For device
// accesses this is not good enough; we need a DMB SY.
//
// Note that the instructions here are the same for AArch64 and ARMv8 AArch32.
#define atomic_device_fence(p)                                                 \
	do {                                                                   \
		switch (p) {                                                   \
		case memory_order_relaxed:                                     \
			atomic_thread_fence(memory_order_relaxed);             \
			break;                                                 \
		case memory_order_acquire:                                     \
		case memory_order_consume:                                     \
			__asm__ volatile("dmb ld" ::: "memory");               \
			break;                                                 \
		case memory_order_release:                                     \
		case memory_order_acq_rel:                                     \
		case memory_order_seq_cst:                                     \
		default:                                                       \
			__asm__ volatile("dmb sy" ::: "memory");               \
			break;                                                 \
		}                                                              \
	} while (0)
