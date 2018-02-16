/*
 * Stress-SGX: Load and stress your enclaves for fun and profit
 * Copyright (C) 2018 Sébastien Vaucher
 * Copyright (C) 2013-2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ENCLAVE_VM_TRUSTED_COMPANION_H_
#define ENCLAVE_VM_TRUSTED_COMPANION_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define OPTIMIZE3 	__attribute__((optimize("-O3")))

#define HOT		__attribute__ ((hot))
#define LIKELY(x)	__builtin_expect((x),1)

#define OPT_FLAGS_MMAP_MINCORE	 0x00000000008000ULL	/* mincore force pages into mem */
#define OPT_FLAGS_VERIFY	 0x00000000002000ULL	/* verify mode */
#define OPT_FLAGS_SGX_VM_KEEP	 0x40000000000000ULL	/* Don't keep re-allocating */

#define VM_BOGO_SHIFT		(12)

#define DEFAULT_VM_HANG		(~0ULL)

#define PAGE_4K_SHIFT		(12)
#define PAGE_4K			(1 << PAGE_4K_SHIFT)

/* Large prime to stride around large VM regions */
#define PRIME_64		(0x8f0000000017116dULL)

extern int mincore_touch_pages(void *buf, const size_t buf_len);

typedef int pid_t;

/* stressor args */
typedef struct {
	uint64_t *const counter;	/* stressor counter */
	const char *name;		/* stressor name */
	const uint64_t max_ops;		/* max number of bogo ops */
	const uint32_t instance;	/* stressor instance # */
	const uint32_t num_instances;	/* number of instances */
	pid_t pid;			/* stressor pid */
	pid_t ppid;			/* stressor ppid */
	size_t page_size;		/* page size */
} args_t;

#if defined(STRESS_X86)
/*
 *  clflush()
 *	flush a cache line
 */
static inline void ALWAYS_INLINE clflush(volatile void *ptr)
{
        asm volatile("clflush %0" : "+m" (*(volatile char *)ptr));
}
#else
#define clflush(ptr)	do { } while (0) /* No-op */
#endif

#endif /* ENCLAVE_VM_TRUSTED_COMPANION_H_ */
