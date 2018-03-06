/*
 * Stress-SGX: Load and stress your enclaves for fun and profit
 * Copyright (C) 2017-2018 Sébastien Vaucher
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "stress-ng.h"
#include "sgx/enclave_vm/trusted/stress-vm.h"

#define VM_BOGO_SHIFT		(12)


typedef struct {
	const char *name;
        const int advice;
} vm_madvise_info_t;

static const vm_madvise_info_t vm_madvise_info[] = {
#if defined(HAVE_MADVISE)
#if defined(MADV_DONTNEED)
	{ "dontneed",	MADV_DONTNEED},
#endif
#if defined(MADV_HUGEPAGE)
	{ "hugepage",	MADV_HUGEPAGE },
#endif
#if defined(MADV_MERGEABLE)
	{ "mergeable",	MADV_MERGEABLE },
#endif
#if defined(MADV_NOHUGEPAGE)
	{ "nohugepage",	MADV_NOHUGEPAGE },
#endif
#if defined(MADV_NORMAL)
	{ "normal",	MADV_NORMAL },
#endif
#if defined(MADV_RANDOM)
	{ "random",	MADV_RANDOM },
#endif
#if defined(MADV_SEQUENTIAL)
	{ "sequential",	MADV_SEQUENTIAL },
#endif
#if defined(MADV_UNMERGEABLE)
	{ "unmergeable",MADV_UNMERGEABLE },
#endif
#if defined(MADV_WILLNEED)
	{ "willneed",	MADV_WILLNEED},
#endif
        { NULL,         0 },
#else
	/* No MADVISE, default to normal, ignored */
	{ "normal",	0 },
#endif
};


void stress_set_vm_hang(const char *opt)
{
	uint64_t vm_hang;

	vm_hang = get_uint64_time(opt);
	check_range("vm-hang", vm_hang,
		MIN_VM_HANG, MAX_VM_HANG);
	set_setting("vm-hang", TYPE_ID_UINT64, &vm_hang);
}

void stress_set_vm_bytes(const char *opt)
{
	size_t vm_bytes;

	vm_bytes = (size_t)get_uint64_byte_memory(opt, 1);
	check_range_bytes("vm-bytes", vm_bytes,
		MIN_VM_BYTES, MAX_MEM_LIMIT);
	set_setting("vm-bytes", TYPE_ID_SIZE_T, &vm_bytes);
}

void stress_set_vm_flags(const int flag)
{
	int vm_flags = 0;

	(void)get_setting("vm-flags", &vm_flags);
	vm_flags |= flag;
	set_setting("vm-flags", TYPE_ID_INT, &vm_flags);

}

int stress_set_vm_madvise(const char *opt)
{
	const vm_madvise_info_t *info;

	for (info = vm_madvise_info; info->name; info++) {
		if (!strcmp(opt, info->name)) {
			set_setting("vm-madvise", TYPE_ID_INT, &info->advice);
			return 0;
		}
	}
	fprintf(stderr, "invalid vm-madvise advice '%s', allowed advice options are:", opt);
	for (info = vm_madvise_info; info->name; info++) {
		fprintf(stderr, " %s", info->name);
        }
	fprintf(stderr, "\n");
	return -1;
}

/*
 *  stress_set_vm_method()
 *      set default vm stress method
 */
int stress_set_vm_method(const char *name)
{
	const int bufsize = 2000;
	int method_exists = 0;

	method_exists = real_vm_method_exists(name);

	if (method_exists == 1) {
		set_setting("vm-method", TYPE_ID_STR, name);
		return 0;
	}

	char methods_error[bufsize];
	memset(methods_error, '\0', bufsize);

	real_get_vm_methods_error(methods_error, bufsize);

	(void)fprintf(stderr, methods_error);
	return -1;
}

/*
 *  stress_vm()
 *	stress virtual memory
 */
int stress_vm(const args_t *args)
{
	uint64_t *bit_error_count = MAP_FAILED;
	uint64_t vm_hang = DEFAULT_VM_HANG;
	uint32_t restarts = 0, nomems = 0;
	size_t vm_bytes = DEFAULT_VM_BYTES;
	char* vm_method;
	pid_t pid;
	const size_t page_size = args->page_size;
	size_t retries;
	int err = 0, ret = EXIT_SUCCESS;
	int vm_madvise = -1;

	(void)get_setting("vm-hang", &vm_hang);
	(void)get_setting("vm-method", &vm_method);
	(void)get_setting("vm-madvise", &vm_madvise);

	pr_dbg("%s using method '%s'\n", args->name, vm_method);

	if (!get_setting("vm-bytes", &vm_bytes)) {
		if (g_opt_flags & OPT_FLAGS_MAXIMIZE)
			vm_bytes = MAX_VM_BYTES;
		if (g_opt_flags & OPT_FLAGS_MINIMIZE)
			vm_bytes = MIN_VM_BYTES;
	}
	vm_bytes /= args->num_instances;
	if (vm_bytes < MIN_VM_BYTES)
		vm_bytes = MIN_VM_BYTES;

	for (retries = 0; (retries < 100) && g_keep_stressing_flag; retries++) {
		bit_error_count = (uint64_t *)
			mmap(NULL, page_size, PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		err = errno;
		if (bit_error_count != MAP_FAILED)
			break;
		(void)shim_usleep(100);
	}

	/* Cannot allocate a single page for bit error counter */
	if (bit_error_count == MAP_FAILED) {
		if (g_keep_stressing_flag) {
			pr_err("%s: could not mmap bit error counter: "
				"retry count=%zu, errno=%d (%s)\n",
				args->name, retries, err, strerror(err));
		}
		return EXIT_NO_RESOURCE;
	}

	*bit_error_count = 0ULL;

again:
	if (!g_keep_stressing_flag)
		goto clean_up;
	pid = fork();
	if (pid < 0) {
		if (errno == EAGAIN)
			goto again;
		pr_err("%s: fork failed: errno=%d: (%s)\n",
			args->name, errno, strerror(errno));
	} else if (pid > 0) {
		int status, waitret;

		/* Parent, wait for child */
		(void)setpgid(pid, g_pgrp);
		waitret = waitpid(pid, &status, 0);
		if (waitret < 0) {
			if (errno != EINTR)
				pr_dbg("%s: waitpid(): errno=%d (%s)\n",
					args->name, errno, strerror(errno));
			(void)kill(pid, SIGTERM);
			(void)kill(pid, SIGKILL);
			(void)waitpid(pid, &status, 0);
		} else if (WIFSIGNALED(status)) {
			pr_dbg("%s: child died: %s (instance %d)\n",
				args->name, stress_strsignal(WTERMSIG(status)),
				args->instance);
			/* If we got killed by OOM killer, re-start */
			if (WTERMSIG(status) == SIGKILL) {
				log_system_mem_info();
				pr_dbg("%s: assuming killed by OOM killer, "
					"restarting again (instance %d)\n",
					args->name, args->instance);
				restarts++;
				goto again;
			}
		}
	} else if (pid == 0) {
		(void)setpgid(0, g_pgrp);
		stress_parent_died_alarm();

		/* Make sure this is killable by OOM killer */
		set_oom_adjustment(args->name, true);

		pr_dbg("Will call into enclave_vm.so\n");
		real_stress_vm(vm_bytes, vm_method, args->max_ops,
				args->counter, &g_keep_stressing_flag, g_opt_flags,
				bit_error_count, page_size, vm_hang, &malloc, &free);

		_exit(EXIT_SUCCESS);
	}
clean_up:
	(void)shim_msync(bit_error_count, page_size, MS_SYNC);
	if (*bit_error_count > 0) {
		pr_fail("%s: detected %" PRIu64 " bit errors while "
			"stressing memory\n",
			args->name, *bit_error_count);
		ret = EXIT_FAILURE;
	}
	(void)munmap((void *)bit_error_count, page_size);

	*args->counter >>= VM_BOGO_SHIFT;

	if (restarts + nomems > 0)
		pr_dbg("%s: OOM restarts: %" PRIu32
			", out of memory restarts: %" PRIu32 ".\n",
			args->name, restarts, nomems);

	return ret;
}
