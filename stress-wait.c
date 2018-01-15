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
 *  Disabled for GNU/Hurd because it this stressor breaks with
 *  the error:
 *    intr-msg.c:387: _hurd_intr_rpc_mach_msg: Assertion 
 *    `m->header.msgh_id == msgid + 100' 
 */
#if !defined(__gnu_hurd__)

#define ABORT_TIMEOUT	(1.0)

static void MLOCKED stress_usr1_handler(int dummy)
{
	(void)dummy;
}

/*
 *  spawn()
 *	spawn a process
 */
static pid_t spawn(
	const args_t *args,
	void (*func)(const args_t *args, const pid_t pid),
	pid_t pid_arg)
{
	pid_t pid;

again:
	pid = fork();
	if (pid < 0) {
		if (g_keep_stressing_flag && (errno == EAGAIN))
			goto again;
		return -1;
	}
	if (pid == 0) {
		//(void)setpgid(0, g_pgrp);
		stress_parent_died_alarm();

		func(args, pid_arg);
		exit(EXIT_SUCCESS);
	}
	(void)setpgid(pid, g_pgrp);
	return pid;
}

/*
 *  runner()
 *	this process pauses, but is continually being
 *	stopped and continued by the killer process
 */
static void runner(
	const args_t *args,
	const pid_t pid)
{
	(void)pid;

	pr_dbg("%s: wait: runner started [%d]\n", args->name, (int)getpid());

	do {
		(void)pause();
	} while (keep_stressing());

	(void)kill(getppid(), SIGALRM);
	exit(EXIT_SUCCESS);
}

/*
 *  killer()
 *	this continually stops and continues the runner process
 */
static void killer(
	const args_t *args,
	const pid_t pid)
{
	double start = time_now();
	uint64_t last_counter = *args->counter;
	pid_t ppid = getppid();

	pr_dbg("%s: wait: killer started [%d]\n", args->name, (int)getpid());

	do {
		(void)kill(pid, SIGSTOP);
		shim_sched_yield();
		(void)kill(pid, SIGCONT);

		/*
		 *  The waits may be blocked and
		 *  so the counter is not being updated.
		 *  If it is blocked for too long bail out
		 *  so we don't get stuck in the parent
		 *  waiter indefinitely.
		 */
		if (last_counter == *args->counter) {
			const double now = time_now();
			if (now - start > ABORT_TIMEOUT) {
				/* unblock waiting parent */
				kill(ppid, SIGUSR1);
				start = now;
			}
		} else {
			start = time_now();
			last_counter = *args->counter;
		}
	} while (keep_stressing());

	/* forcefully kill runner, wait is in parent */
	(void)kill(pid, SIGKILL);
	/* tell parent to wake up! */
	(void)kill(getppid(), SIGALRM);
	exit(EXIT_SUCCESS);
}

/*
 *  stress_wait
 *	stress wait*() family of calls
 */
int stress_wait(const args_t *args)
{
	int status, ret = EXIT_SUCCESS;
	pid_t pid_r, pid_k, wret;
	int options = 0;

#if defined(WUNTRACED)
	options |= WUNTRACED;
#endif
#if defined(WCONTINUED)
	options |= WCONTINUED;
#endif

	pr_dbg("%s: waiter started [%d]\n",
		args->name, (int)args->pid);

	if (stress_sighandler(args->name, SIGUSR1, stress_usr1_handler, NULL) < 0)
		return EXIT_FAILURE;

	pid_r = spawn(args, runner, 0);
	if (pid_r < 0) {
		pr_fail_dbg("fork");
		exit(EXIT_FAILURE);
	}

	pid_k = spawn(args, killer, pid_r);
	if (pid_k < 0) {
		pr_fail_dbg("fork");
		ret = EXIT_FAILURE;
		goto tidy;
	}

	do {
		wret = waitpid(pid_r, &status, options);
		if ((wret < 0) && (errno != EINTR) && (errno != ECHILD)) {
			pr_fail_dbg("waitpid()");
			break;
		}
		if (!g_keep_stressing_flag)
			break;
#if defined(WIFCONINUED)
		if (WIFCONTINUED(status))
			inc_counter(args);
#else
		inc_counter(args);
#endif

#if defined(WCONTINUED) && \
    (_SVID_SOURCE || _XOPEN_SOURCE >= 500 || \
    _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED || \
    _POSIX_C_SOURCE >= 200809L)
		if (options) {
			siginfo_t info;

			wret = waitid(P_PID, pid_r, &info, options);
			if ((wret < 0) && (errno != EINTR) && (errno != ECHILD)) {
				pr_fail_dbg("waitpid()");
				break;
			}
			if (!g_keep_stressing_flag)
				break;
#if defined(WIFCONTINUED)
			if (WIFCONTINUED(status))
				inc_counter(args);
#else
			inc_counter(args);
#endif
		}
#endif
	} while (g_keep_stressing_flag && (!args->max_ops || *args->counter < args->max_ops));

	(void)kill(pid_k, SIGKILL);
	(void)waitpid(pid_k, &status, 0);
tidy:
	(void)kill(pid_r, SIGKILL);
	(void)waitpid(pid_r, &status, 0);

	return ret;
}
#else
int stress_wait(const args_t *args)
{
	return stress_not_implemented(args);
}
#endif
