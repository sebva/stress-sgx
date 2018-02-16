/*
 * Stress-SGX: Load and stress your enclaves for fun and profit
 * Copyright (C) 2017-2018 Sébastien Vaucher
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
#include "companion.h"

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


void pr_dbg(const char *fmt, ...)
{
	int ret = 0;
	char buf[BUFSIZ] = {'\0'};
	va_list ap;

	va_start(ap, fmt);
	ret = vsnprintf(buf, BUFSIZ, fmt, ap);
	if (ret >= 0) {
		ocall_pr_dbg(buf);
	}
	va_end(ap);
}

void pr_fail(const char *fmt, ...)
{
	int ret = 0;
	char buf[BUFSIZ] = {'\0'};
	va_list ap;

	va_start(ap, fmt);
	ret = vsnprintf(buf, BUFSIZ, fmt, ap);
	if (ret >= 0) {
		ocall_pr_fail(buf);
	}
	va_end(ap);
}
