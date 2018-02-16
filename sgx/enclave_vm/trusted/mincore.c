/*
 * Stress-SGX: Load and stress your enclaves for fun and profit
 * Copyright (C) 2018 Sébastien Vaucher
 * Copyright (C) 2013-2018 Canonical, Ltd.
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
#include "stress-vm.h"
#include "companion.h"
#include "stress-version.h"

/*
 *  stress_get_pagesize()
 *	get pagesize
 */
size_t stress_get_pagesize(void)
{
#if defined(_SC_PAGESIZE)
	long sz;
#endif
	static size_t page_size = 0;

	if (page_size > 0)
		return page_size;

#if defined(_SC_PAGESIZE)
	sz = sysconf(_SC_PAGESIZE);
	page_size = (sz <= 0) ? PAGE_4K : (size_t)sz;
#else
	page_size = PAGE_4K;
#endif

	return page_size;
}

/*
 *  shim_mincore()
 *	wrapper for mincore(2) -  determine whether pages are resident in memory
 */
int shim_mincore(void *addr, size_t length, unsigned char *vec)
{
#if defined(HAVE_MINCORE) && NEED_GLIBC(2,2,0)
#if defined(__FreeBSD__) || defined(__OpenBSD__) || \
    defined(__NetBSD__) || defined(__sun__)
	return mincore(addr, length, (char *)vec);
#else
	return mincore(addr, length, vec);
#endif
#else
	(void)addr;
	(void)length;
	(void)vec;

	//errno = ENOSYS;
	return -1;
#endif
}

/*
 * mincore_touch_pages_slow()
 *	touch pages, even when they are resident
 */
static void mincore_touch_pages_slow(
	void *buf,
	const size_t n_pages,
	const size_t page_size)
{
	size_t i;
	volatile char *buffer;

	for (buffer = buf, i = 0; i < n_pages; i++, buffer += page_size)
		(*buffer)++;
	for (buffer = buf, i = 0; i < n_pages; i++, buffer += page_size)
		(*buffer)--;
}

/*
 * mincore_touch_pages()
 *	touch a range of pages, ensure they are all in memory
 */
int mincore_touch_pages(void *buf, const size_t buf_len)
{
	const size_t page_size = stress_get_pagesize();
	const size_t n_pages = buf_len / page_size;

#if !defined(HAVE_MINCORE)
	/* systems that don't have mincore */
	mincore_touch_pages_slow(buf, n_pages, page_size);
	return 0;
#else
	/* systems that support mincore */
	unsigned char *vec;
	volatile char *buffer;
	size_t i;
	uintptr_t uintptr = (uintptr_t)buf & (page_size - 1);

	if (!(g_opt_flags & OPT_FLAGS_MMAP_MINCORE))
		return 0;
	if (n_pages < 1)
		return -1;

	vec = calloc(n_pages, 1);
	if (!vec) {
		mincore_touch_pages_slow(buf, n_pages, page_size);
		return 0;
	}

	/*
	 *  Find range of pages that are not in memory
	 */
	if (shim_mincore((void *)uintptr, buf_len, vec) < 0) {
		free(vec);

		mincore_touch_pages_slow(buf, n_pages, page_size);
		return 0;
	}

	/* If page is not resident in memory, touch it */
	for (buffer = buf, i = 0; i < n_pages; i++, buffer += page_size)
		if (!(vec[i] & 1))
			(*buffer)++;

	/* And restore contents */
	for (buffer = buf, i = 0; i < n_pages; i++, buffer += page_size)
		if (!(vec[i] & 1))
			(*buffer)--;
	free(vec);
	return 0;
#endif
}
