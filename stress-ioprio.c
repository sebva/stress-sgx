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

#if defined(__linux__) && defined(__NR_ioprio_set) && defined(__NR_ioprio_get)

#include <sys/uio.h>

#define MAX_IOV		(4)
#define BUF_SIZE	(32)

/*
 *  stress set/get io priorities
 *	stress system by rapid io priority changes
 */
int stress_ioprio(const args_t *args)
{
	const uid_t uid = getuid();
#if defined(HAVE_GETPGRP)
	const pid_t grp = getpgrp();
#endif
	int fd, rc = EXIT_FAILURE;
	char filename[PATH_MAX];

	if (stress_temp_dir_mk_args(args) < 0)
		return rc;

	(void)stress_temp_filename_args(args,
		filename, sizeof(filename), mwc32());
	(void)umask(0077);
	if ((fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR)) < 0) {
		rc = exit_status(errno);
		pr_fail_err("open");
		goto cleanup_dir;
	}
	(void)unlink(filename);

	do {
		int i;
		struct iovec iov[MAX_IOV];
		char buffer[MAX_IOV][BUF_SIZE];

		if (shim_ioprio_get(IOPRIO_WHO_PROCESS, args->pid) < 0) {
			pr_fail("%s: ioprio_get(OPRIO_WHO_PROCESS, %d), "
				"errno = %d (%s)\n",
				args->name, args->pid, errno, strerror(errno));
			goto cleanup_file;
		}
		if (shim_ioprio_get(IOPRIO_WHO_PROCESS, 0) < 0) {
			pr_fail("%s: ioprio_get(OPRIO_WHO_PROCESS, 0), "
				"errno = %d (%s)\n",
				args->name, errno, strerror(errno));
			goto cleanup_file;
		}
#if defined(HAVE_GETPGRP)
		if (shim_ioprio_get(IOPRIO_WHO_PGRP, grp) < 0) {
			pr_fail("%s: ioprio_get(OPRIO_WHO_PGRP, %d), "
				"errno = %d (%s)\n",
				args->name, g_pgrp, errno, strerror(errno));
			goto cleanup_file;
		}
#endif
		if (shim_ioprio_get(IOPRIO_WHO_PGRP, 0) < 0) {
			pr_fail("%s: ioprio_get(OPRIO_WHO_PGRP, 0), "
				"errno = %d (%s)\n",
				args->name, errno, strerror(errno));
			goto cleanup_file;
		}
		if (shim_ioprio_get(IOPRIO_WHO_USER, uid) < 0) {
			pr_fail("%s: ioprio_get(OPRIO_WHO_USR, %d), "
				"errno = %d (%s)\n",
				args->name, uid, errno, strerror(errno));
			goto cleanup_file;
		}

		for (i = 0; i < MAX_IOV; i++) {
			(void)memset(buffer[i], mwc8(), BUF_SIZE);
			iov[i].iov_base = buffer[i];
			iov[i].iov_len = BUF_SIZE;
		}

		if (pwritev(fd, iov, MAX_IOV, (off_t)512 * mwc16()) < 0) {
			pr_fail_err("pwritev");
			goto cleanup_file;
		}
		(void)fsync(fd);

		if (shim_ioprio_set(IOPRIO_WHO_PROCESS, args->pid,
			IOPRIO_PRIO_VALUE(IOPRIO_CLASS_IDLE, 0)) < 0) {
			if (errno != EPERM) {
				pr_fail("%s: ioprio_set("
					"IOPRIO_WHO_PROCESS, %d, "
					"(IOPRIO_CLASS_IDLE, 0)), "
					"errno = %d (%s)\n",
					args->name, args->pid, errno, strerror(errno));
				goto cleanup_file;
			}
		}

		if (pwritev(fd, iov, MAX_IOV, (off_t)512 * mwc16()) < 0) {
			pr_fail_err("pwritev");
			goto cleanup_file;
		}
		(void)fsync(fd);

		for (i = 0; i < 8; i++) {
			if (shim_ioprio_set(IOPRIO_WHO_PROCESS, args->pid,
				IOPRIO_PRIO_VALUE(IOPRIO_CLASS_BE, i)) < 0) {
				if (errno != EPERM) {
					pr_fail("%s: ioprio_set("
						"IOPRIO_WHO_PROCESS, %d, "
						"(IOPRIO_CLASS_BE, %d)), "
						"errno = %d (%s)\n",
						args->name, args->pid, i, errno, strerror(errno));
					goto cleanup_file;
				}
			}
			if (pwritev(fd, iov, MAX_IOV, (off_t)512 * mwc16()) < 0) {
				pr_fail_err("pwritev");
				goto cleanup_file;
			}
			(void)fsync(fd);
		}
		for (i = 0; i < 8; i++) {
			if (shim_ioprio_set(IOPRIO_WHO_PROCESS, args->pid,
				IOPRIO_PRIO_VALUE(IOPRIO_CLASS_RT, i)) < 0) {
				if (errno != EPERM) {
					pr_fail("%s: ioprio_set("
						"IOPRIO_WHO_PROCESS, %d, "
						"(IOPRIO_CLASS_RT, %d)), "
						"errno = %d (%s)\n",
						args->name, args->pid, i, errno, strerror(errno));
					goto cleanup_file;
				}
			}
			if (pwritev(fd, iov, MAX_IOV, (off_t)512 * mwc16()) < 0) {
				pr_fail_err("pwritev");
				goto cleanup_file;
			}
			(void)fsync(fd);
		}
		inc_counter(args);
	} while (keep_stressing());

	rc = EXIT_SUCCESS;

cleanup_file:
	(void)close(fd);
cleanup_dir:
	(void)stress_temp_dir_rm_args(args);

	return rc;
}
#else
int stress_ioprio(const args_t *args)
{
	return stress_not_implemented(args);
}
#endif
