/*
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
 *
 * This code is a complete clean re-write of the stress tool by
 * Colin Ian King <colin.king@canonical.com> and attempts to be
 * backwardly compatible with the stress tool by Amos Waterland
 * <apw@rossby.metr.ou.edu> but has more stress tests and more
 * functionality.
 *
 */
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>

//#include "complex.h"
#define complex	_Complex
#define I 1i
double complex cexp(double complex z);
double cabs(double complex z);

#define M_PI		3.14159265358979323846	/* pi */
#define M_E		2.7182818284590452354	/* e */
#define FFT_SIZE		(4096)
#define STRESS_CPU_DITHER_X	(1024)
#define STRESS_CPU_DITHER_Y	(768)


#define CRT_INFINITY __builtin_huge_valf()
#define crt_isinf(x) __builtin_isinf((x))
#define crt_isnan(x) __builtin_isnan((x))
#define crt_copysign(x, y) __builtin_copysign((x), (y))
double _Complex __muldc3(double __a, double __b, double __c, double __d)
{
    double __ac = __a * __c;
    double __bd = __b * __d;
    double __ad = __a * __d;
    double __bc = __b * __c;
    double _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (crt_isnan(__real__ z) && crt_isnan(__imag__ z))
    {
        int __recalc = 0;
        if (crt_isinf(__a) || crt_isinf(__b))
        {
            __a = crt_copysign(crt_isinf(__a) ? 1 : 0, __a);
            __b = crt_copysign(crt_isinf(__b) ? 1 : 0, __b);
            if (crt_isnan(__c))
                __c = crt_copysign(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysign(0, __d);
            __recalc = 1;
        }
        if (crt_isinf(__c) || crt_isinf(__d))
        {
            __c = crt_copysign(crt_isinf(__c) ? 1 : 0, __c);
            __d = crt_copysign(crt_isinf(__d) ? 1 : 0, __d);
            if (crt_isnan(__a))
                __a = crt_copysign(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysign(0, __b);
            __recalc = 1;
        }
        if (!__recalc && (crt_isinf(__ac) || crt_isinf(__bd) ||
                          crt_isinf(__ad) || crt_isinf(__bc)))
        {
            if (crt_isnan(__a))
                __a = crt_copysign(0, __a);
            if (crt_isnan(__b))
                __b = crt_copysign(0, __b);
            if (crt_isnan(__c))
                __c = crt_copysign(0, __c);
            if (crt_isnan(__d))
                __d = crt_copysign(0, __d);
            __recalc = 1;
        }
        if (__recalc)
        {
            __real__ z = CRT_INFINITY * (__a * __c - __b * __d);
            __imag__ z = CRT_INFINITY * (__a * __d + __b * __c);
        }
    }

    return z;
}

#define STRESS_VECTOR	1
#define CASE_FALLTHROUGH __attribute__((fallthrough)) /* Fallthrough */
#define NORETURN 	__attribute__ ((noreturn))
#define ALWAYS_INLINE	__attribute__ ((always_inline))
#define NOINLINE	__attribute__ ((noinline))
#define OPTIMIZE3 	__attribute__((optimize("-O3")))
#define OPTIMIZE1 	__attribute__((optimize("-O1")))
#define WARN_UNUSED	__attribute__((warn_unused_result))
#define ALIGN128	__attribute__ ((aligned(128)))
#define ALIGN64		__attribute__ ((aligned(64)))
#if defined(ALIGN128)
#define ALIGN_CACHELINE ALIGN128
#else
#define ALIGN_CACHELINE ALIGN64
#endif

#define HOT		__attribute__ ((hot))
#define MLOCKED		__attribute__((__section__("mlocked")))
#define MLOCKED_SECTION 1
#define FORMAT(func, a, b) __attribute__((format(func, a, b)))
#define RESTRICT __restrict
#define LIKELY(x)	__builtin_expect((x),1)
#define UNLIKELY(x)	__builtin_expect((x),0)
#define SIZEOF_ARRAY(a)		(sizeof(a) / sizeof(a[0]))

/* waste some cycles */
#define FORCE_DO_NOTHING() __asm__ __volatile__("nop;")

/*
 *  externs to force gcc to stash computed values and hence
 *  to stop the optimiser optimising code away to zero. The
 *  *_put funcs are essentially no-op functions.
 */
extern uint64_t uint64_zero(void);

static volatile uint64_t uint64_val;
static volatile double   double_val;

/*
 *  uint64_put()
 *	stash a uint64_t value
 */
static inline void ALWAYS_INLINE uint64_put(const uint64_t a)
{
	uint64_val = a;
}

/*
 *  double_put()
 *	stash a double value
 */
static inline void ALWAYS_INLINE double_put(const double a)
{
	double_val = a;
}

/* MWC random number initial seed */
#define MWC_SEED_Z		(362436069UL)
#define MWC_SEED_W		(521288629UL)
#define MWC_SEED()		mwc_seed(MWC_SEED_W, MWC_SEED_Z)

/* Fast random number generator state */
typedef struct {
	uint32_t w;
	uint32_t z;
} mwc_t;

static mwc_t __mwc = {
	MWC_SEED_W,
	MWC_SEED_Z
};

static uint8_t mwc_n8, mwc_n16;

static inline void mwc_flush(void)
{
	mwc_n8 = 0;
	mwc_n16 = 0;
}

/*
 *  mwc32()
 *      Multiply-with-carry random numbers
 *      fast pseudo random number generator, see
 *      http://www.cse.yorku.ca/~oz/marsaglia-rng.html
 */
HOT OPTIMIZE3 uint32_t mwc32(void)
{
	__mwc.z = 36969 * (__mwc.z & 65535) + (__mwc.z >> 16);
	__mwc.w = 18000 * (__mwc.w & 65535) + (__mwc.w >> 16);
	return (__mwc.z << 16) + __mwc.w;
}

/*
 *  mwc_reseed()
 *	dirty mwc reseed
 */
void mwc_reseed(void)
{
	__mwc.w = MWC_SEED_W;
	__mwc.z = MWC_SEED_Z;
	mwc_flush();
}

/*
 *  mwc_seed()
 *      set mwc seeds
 */
void mwc_seed(const uint32_t w, const uint32_t z)
{
	__mwc.w = w;
	__mwc.z = z;

	mwc_flush();
}

/*
 *  mwc64()
 *	get a 64 bit pseudo random number
 */
HOT OPTIMIZE3 uint64_t mwc64(void)
{
	return (((uint64_t)mwc32()) << 32) | mwc32();
}

/*
 *  mwc16()
 *	get a 16 bit pseudo random number
 */
HOT OPTIMIZE3 uint16_t mwc16(void)
{
	static uint32_t mwc_saved;

	if (mwc_n16) {
		mwc_n16--;
		mwc_saved >>= 16;
	} else {
		mwc_n16 = 1;
		mwc_saved = mwc32();
	}
	return mwc_saved & 0xffff;
}

/*
 *  mwc8()
 *	get an 8 bit pseudo random number
 */
HOT OPTIMIZE3 uint8_t mwc8(void)
{
	static uint32_t mwc_saved;

	if (LIKELY(mwc_n8)) {
		mwc_n8--;
		mwc_saved >>= 8;
	} else {
		mwc_n8 = 3;
		mwc_saved = mwc32();
	}
	return mwc_saved & 0xff;
}


typedef int pid_t;

struct timeval
{
  long int tv_sec;              /* Seconds.  */
  long int tv_usec;        /* Microseconds.  */
};

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


