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

#if defined(__linux__) && defined(__NR_name_to_handle_at) && \
    defined(__NR_open_by_handle_at) && NEED_GLIBC(2,14,0)

#define MAX_MOUNT_IDS	(1024)
#define FILENAME	"/dev/zero"

/* Stringification macros */
#define XSTR(s)	STR(s)
#define STR(s) #s

typedef struct {
	char	*mount_path;
	int	mount_id;
} mount_info_t;

static mount_info_t mount_info[MAX_MOUNT_IDS];

static void free_mount_info(const int mounts)
{
	int i;

	for (i = 0; i < mounts; i++)
		free(mount_info[i].mount_path);
}

static int get_mount_info(const args_t *args)
{
	FILE *fp;
	int mounts = 0;

	if ((fp = fopen("/proc/self/mountinfo", "r")) == NULL) {
		pr_dbg("%s: cannot open /proc/self/mountinfo\n", args->name);
		return -1;
	}

	for (;;) {
		char mount_path[PATH_MAX + 1];
		char *line = NULL;
		size_t line_len = 0;

		ssize_t nread = getline(&line, &line_len, fp);
		if (nread == -1) {
			free(line);
			break;
		}

		nread = sscanf(line, "%12d %*d %*s %*s %" XSTR(PATH_MAX) "s",
			&mount_info[mounts].mount_id,
			mount_path);
		free(line);
		if (nread != 2)
			continue;

		mount_info[mounts].mount_path = strdup(mount_path);
		if (mount_info[mounts].mount_path == NULL) {
			pr_dbg("%s: cannot allocate mountinfo mount path\n", args->name);
			free_mount_info(mounts);
			mounts = -1;
			break;
		}
		mounts++;
	}
	(void)fclose(fp);
	return mounts;
}


/*
 *  stress_handle()
 *	stress system by rapid open/close calls via
 *	name_to_handle_at and open_by_handle_at
 */
int stress_handle(const args_t *args)
{
	int mounts;

	if ((mounts = get_mount_info(args)) < 0) {
		pr_fail("%s: failed to parse /proc/self/mountinfo\n", args->name);
		return EXIT_FAILURE;
	}

	do {
		struct file_handle *fhp, *tmp;
		int mount_id, mount_fd, fd, i;

		if ((fhp = malloc(sizeof(*fhp))) == NULL)
			continue;

		fhp->handle_bytes = 0;
		if ((name_to_handle_at(AT_FDCWD, FILENAME, fhp, &mount_id, 0) != -1) &&
		    (errno != EOVERFLOW)) {
			pr_fail_err("name_to_handle_at: failed to get file handle size");
			free(fhp);
			break;
		}
		tmp = realloc(fhp, sizeof(struct file_handle) + fhp->handle_bytes);
		if (tmp == NULL) {
			free(fhp);
			continue;
		}
		fhp = tmp;
		if (name_to_handle_at(AT_FDCWD, FILENAME, fhp, &mount_id, 0) < 0) {
			pr_fail_err("name_to_handle_at: failed to get file handle");
			free(fhp);
			break;
		}

		mount_fd = -2;
		for (i = 0; i < mounts; i++) {
			if (mount_info[i].mount_id == mount_id) {
				mount_fd = open(mount_info[i].mount_path, O_RDONLY);
				break;
			}
		}
		if (mount_fd == -2) {
			pr_fail("%s: cannot find mount id %d\n", args->name, mount_id);
			free(fhp);
			break;
		}
		if (mount_fd < 0) {
			pr_fail("%s: failed to open mount path '%s': errno=%d (%s)\n",
				args->name, mount_info[i].mount_path, errno, strerror(errno));
			free(fhp);
			break;
		}
		if ((fd = open_by_handle_at(mount_fd, fhp, O_RDONLY)) < 0) {
			/* We don't abort if EPERM occurs, that's not a test failure */
			if (errno != EPERM) {
				pr_fail("%s: open_by_handle_at: failed to open: errno=%d (%s)\n",
					args->name, errno, strerror(errno));
				(void)close(mount_fd);
				free(fhp);
				break;
			}
		} else {
			(void)close(fd);
		}
		(void)close(mount_fd);
		free(fhp);
		inc_counter(args);
	} while (keep_stressing());

	free_mount_info(mounts);

	return EXIT_SUCCESS;
}
#else
int stress_handle(const args_t *args)
{
	return stress_not_implemented(args);
}
#endif
