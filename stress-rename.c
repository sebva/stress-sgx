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

/*
 *  stress_rename()
 *	stress system by renames
 */
int stress_rename(const args_t *args)
{
	char name1[PATH_MAX], name2[PATH_MAX];
	char *oldname = name1, *newname = name2, *tmpname;
	FILE *fp;
	uint64_t i = 0;
	const uint32_t inst1 = args->instance * 2;
	const uint32_t inst2 = inst1 + 1;

	if (stress_temp_dir_mk(args->name, args->pid, inst1) < 0)
		return EXIT_FAILURE;
	if (stress_temp_dir_mk(args->name, args->pid, inst2) < 0) {
		(void)stress_temp_dir_rm(args->name, args->pid, inst1);
		return EXIT_FAILURE;
	}
restart:
	(void)stress_temp_filename(oldname, PATH_MAX,
		args->name, args->pid, inst1, i++);

	if ((fp = fopen(oldname, "w+")) == NULL) {
		int rc = exit_status(errno);
		pr_err("%s: fopen failed: errno=%d: (%s)\n",
			args->name, errno, strerror(errno));
		(void)stress_temp_dir_rm(args->name, args->pid, inst1);
		(void)stress_temp_dir_rm(args->name, args->pid, inst2);
		return rc;
	}
	(void)fclose(fp);

	for (;;) {
		(void)stress_temp_filename(newname, PATH_MAX,
			args->name, args->pid, inst2, i++);
		if (rename(oldname, newname) < 0) {
			(void)unlink(oldname);
			(void)unlink(newname);
			goto restart;
		}

		tmpname = oldname;
		oldname = newname;
		newname = tmpname;
		inc_counter(args);
		if (!keep_stressing())
			break;

		(void)stress_temp_filename(newname, PATH_MAX,
			args->name, args->pid, inst1, i++);
		if (rename(oldname, newname) < 0) {
			(void)unlink(oldname);
			(void)unlink(newname);
			goto restart;
		}

		tmpname = oldname;
		oldname = newname;
		newname = tmpname;
		inc_counter(args);
		if (!keep_stressing())
			break;
	}

	(void)unlink(oldname);
	(void)unlink(newname);
	(void)stress_temp_dir_rm(args->name, args->pid, inst1);
	(void)stress_temp_dir_rm(args->name, args->pid, inst2);

	return EXIT_SUCCESS;
}
