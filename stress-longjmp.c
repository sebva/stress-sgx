/*
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
#include "stress-ng.h"

static jmp_buf buf;

static void OPTIMIZE1 NOINLINE NORETURN stress_longjmp_func(void)
{
	longjmp(buf, 1);	/* Jump out */

	_exit(EXIT_FAILURE);	/* Never get here */
}

/*
 *  stress_jmp()
 *	stress system by setjmp/longjmp calls
 */
int OPTIMIZE1 stress_longjmp(const args_t *args)
{
	int ret;

	ret = setjmp(buf);

	if (ret) {
		static int c = 0;

		c++;
		if (c >= 1000) {
			inc_counter(args);
			c = 0;
		}
	}
	if (keep_stressing())
		stress_longjmp_func();

	return EXIT_SUCCESS;
}
