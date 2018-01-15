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

#define CLONE_STACK_SIZE	(16*1024)

typedef struct clone {
	pid_t	pid;
	struct clone *next;
	char stack[CLONE_STACK_SIZE];
} clone_t;

typedef struct {
	clone_t *head;		/* Head of clone procs list */
	clone_t *tail;		/* Tail of clone procs list */
	clone_t *free;		/* List of free'd clones */
	uint64_t length;	/* Length of list */
} clone_list_t;

#if defined(__linux__) && NEED_GLIBC(2,14,0)

static clone_list_t clones;

/*
 *  A random selection of clone flags that are worth exercising
 */
static const int flags[] = {
	0,
#if defined(CLONE_FILES)
	CLONE_FILES,
#endif
#if defined(CLONE_FS)
	CLONE_FS,
#endif
#if defined(CLONE_IO)
	CLONE_IO,
#endif
#if defined(CLONE_NEWIPC)
	CLONE_NEWIPC,
#endif
#if defined(CLONE_NEWNET)
	CLONE_NEWNET,
#endif
#if defined(CLONE_NEWNS)
	CLONE_NEWNS,
#endif
#if defined(CLONE_NEWUSER)
	CLONE_NEWUSER,
#endif
#if defined(CLONE_NEWUTS)
	CLONE_NEWUTS,
#endif
#if defined(CLONE_SIGHAND)
	CLONE_SIGHAND,
#endif
#if defined(CLONE_SYSVSEM)
	CLONE_SYSVSEM,
#endif
#if defined(CLONE_UNTRACED)
	CLONE_UNTRACED,
#endif
};

static const int unshare_flags[] = {
#if defined(CLONE_FILES)
	CLONE_FILES,
#endif
#if defined(CLONE_FS)
	CLONE_FS,
#endif
#if defined(CLONE_NEWIPC)
	CLONE_NEWIPC,
#endif
#if defined(CLONE_NEWNET)
	CLONE_NEWNET,
#endif
#if defined(CLONE_NEWNS)
	CLONE_NEWNS,
#endif
#if defined(CLONE_NEWUTS)
	CLONE_NEWUTS,
#endif
#if defined(CLONE_SYSVSEM)
	CLONE_SYSVSEM,
#endif
};
#endif

/*
 *  stress_set_clone_max()
 *	set maximum number of clones allowed
 */
void stress_set_clone_max(const char *opt)
{
	uint64_t clone_max;

	clone_max = get_uint64(opt);
	check_range("clone-max", clone_max,
		MIN_ZOMBIES, MAX_ZOMBIES);
	set_setting("clone-max", TYPE_ID_UINT64, &clone_max);
}

#if defined(__linux__) && NEED_GLIBC(2,14,0)

/*
 *  stress_clone_new()
 *	allocate a new clone, add to end of list
 */
static clone_t *stress_clone_new(void)
{
	clone_t *new;

	if (clones.free) {
		/* Pop an old one off the free list */
		new = clones.free;
		clones.free = new->next;
		new->next = NULL;
	} else {
		new = calloc(1, sizeof(*new));
		if (!new)
			return NULL;
	}

	if (clones.head)
		clones.tail->next = new;
	else
		clones.head = new;

	clones.tail = new;
	clones.length++;

	return new;
}

/*
 *  stress_clone_head_remove
 *	reap a clone and remove a clone from head of list, put it onto
 *	the free clone list
 */
static void stress_clone_head_remove(void)
{
	if (clones.head) {
		int status;
		clone_t *head = clones.head;

		(void)waitpid(clones.head->pid, &status, __WCLONE);

		if (clones.tail == clones.head) {
			clones.tail = NULL;
			clones.head = NULL;
		} else {
			clones.head = head->next;
		}

		/* Shove it on the free list */
		head->next = clones.free;
		clones.free = head;

		clones.length--;
	}
}

/*
 *  stress_clone_free()
 *	free the clones off the clone free lists
 */
static void stress_clone_free(void)
{
	while (clones.head) {
		clone_t *next = clones.head->next;

		free(clones.head);
		clones.head = next;
	}
	while (clones.free) {
		clone_t *next = clones.free->next;

		free(clones.free);
		clones.free = next;
	}
}

/*
 *  clone_func()
 *	clone thread just returns immediately
 */
static int clone_func(void *arg)
{
	size_t i;

	(void)arg;

	for (i = 0; i < SIZEOF_ARRAY(unshare_flags); i++) {
		(void)unshare(unshare_flags[i]);
	}

	return 0;
}

/*
 *  stress_clone()
 *	stress by cloning and exiting
 */
int stress_clone(const args_t *args)
{
	uint64_t max_clones = 0;
	uint64_t clone_max = DEFAULT_ZOMBIES;
	pid_t pid;
	const ssize_t stack_offset =
		stress_get_stack_direction() *
		(CLONE_STACK_SIZE - 64);

	if (!get_setting("clone-max", &clone_max)) {
		if (g_opt_flags & OPT_FLAGS_MAXIMIZE)
			clone_max = MAX_ZOMBIES;
		if (g_opt_flags & OPT_FLAGS_MINIMIZE)
			clone_max = MIN_ZOMBIES;
	}

again:
	if (!g_keep_stressing_flag)
		return EXIT_SUCCESS;
	pid = fork();
	if (pid < 0) {
		if (errno == EAGAIN)
			goto again;
		pr_err("%s: fork failed: errno=%d: (%s)\n",
			args->name, errno, strerror(errno));
	} else if (pid > 0) {
		int status, ret;

		(void)setpgid(pid, g_pgrp);
		/* Parent, wait for child */
		ret = waitpid(pid, &status, 0);
		if (ret < 0) {
			if (errno != EINTR)
				pr_dbg("%s: waitpid(): errno=%d (%s)\n",
					args->name, errno, strerror(errno));
			(void)kill(pid, SIGALRM);
			(void)waitpid(pid, &status, 0);
			/* And kill it for sure */
			(void)kill(pid, SIGKILL);
		} else if (WIFSIGNALED(status)) {
			pr_dbg("%s: child died: %s (instance %d)\n",
				args->name, stress_strsignal(WTERMSIG(status)),
				args->instance);
			/* If we got killed by OOM killer, re-start */
			if ((WTERMSIG(status) == SIGKILL) ||
			    (WTERMSIG(status) == SIGTERM)) {
				if (g_opt_flags & OPT_FLAGS_OOMABLE) {
					log_system_mem_info();
					pr_dbg("%s: assuming killed by OOM "
						"killer, bailing out "
						"(instance %d)\n",
						args->name, args->instance);
					_exit(0);
				} else {
					log_system_mem_info();
					pr_dbg("%s: assuming killed by OOM "
						"killer, restarting again "
						"(instance %d)\n",
						args->name, args->instance);
					goto again;
				}
			}
		}
	} else if (pid == 0) {
		/* Child */

		(void)setpgid(0, g_pgrp);
		stress_parent_died_alarm();

		/* Make sure this is killable by OOM killer */
		set_oom_adjustment(args->name, true);

		do {
			if (clones.length < clone_max) {
				clone_t *clone_info;
				char *stack_top;
				int flag = flags[mwc32() % SIZEOF_ARRAY(flags)];

				clone_info = stress_clone_new();
				if (!clone_info)
					break;
				stack_top = clone_info->stack + stack_offset;
				clone_info->pid = clone(clone_func,
					align_stack(stack_top), flag, NULL);
				if (clone_info->pid == -1) {
					/*
					 * Reached max forks or error
					 * (e.g. EPERM)? .. then reap
					 */
					stress_clone_head_remove();
					continue;
				}
				if (max_clones < clones.length)
					max_clones = clones.length;
				inc_counter(args);
			} else {
				stress_clone_head_remove();
			}
		} while (keep_stressing());

		pr_inf("%s: created a maximum of %" PRIu64 " clones\n",
			args->name, max_clones);
		/* And reap */
		while (clones.head) {
			stress_clone_head_remove();
		}
		/* And free */
		stress_clone_free();

		_exit(0);
	}

	return EXIT_SUCCESS;
}
#else
int stress_clone(const args_t *args)
{
	return stress_not_implemented(args);
}
#endif
