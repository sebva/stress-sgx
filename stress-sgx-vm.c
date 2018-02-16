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
#include "sgx/utils.h"
#include "sgx/enclave_vm/untrusted/vm_u.h"

typedef struct {
	const char *name;
        const int advice;
} vm_madvise_info_t;

#define VM_BOGO_SHIFT		(12)

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



void stress_set_sgx_vm_hang(const char *opt)
{
	uint64_t vm_hang;

	vm_hang = get_uint64_time(opt);
	check_range("sgx-vm-hang", vm_hang,
		MIN_VM_HANG, MAX_VM_HANG);
	set_setting("vm-hang", TYPE_ID_UINT64, &vm_hang);
}

void stress_set_sgx_vm_bytes(const char *opt)
{
	size_t vm_bytes;

	vm_bytes = (size_t)get_uint64_byte_memory(opt, 1);
	check_range_bytes("sgx-vm-bytes", vm_bytes,
		MIN_VM_BYTES, MAX_MEM_LIMIT);
	set_setting("sgx-vm-bytes", TYPE_ID_SIZE_T, &vm_bytes);
}

void stress_set_sgx_vm_flags(const int flag)
{
	int vm_flags = 0;

	(void)get_setting("sgx-vm-flags", &vm_flags);
	vm_flags |= flag;
	set_setting("sgx-vm-flags", TYPE_ID_INT, &vm_flags);

}

int stress_set_sgx_vm_madvise(const char *opt)
{
	const vm_madvise_info_t *info;

	for (info = vm_madvise_info; info->name; info++) {
		if (!strcmp(opt, info->name)) {
			set_setting("sgx-vm-madvise", TYPE_ID_INT, &info->advice);
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
int stress_set_sgx_vm_method(const char *name)
{
	const int bufsize = 2000;
	int method_exists = 0;
	sgx_enclave_id_t eid = 0;
	sgx_status_t status;

	/* Initialize the enclave */
	status = initialize_enclave(&eid, ENCLAVE_VM_FILENAME, TOKEN_VM_FILENAME);
	if (status != SGX_SUCCESS) {
		printf("Error %d\n", status);
		return -1;
	}

	status = ecall_vm_method_exists(eid, &method_exists, name);
	if (status != SGX_SUCCESS) {
		pr_fail("Unable to enter enclave, check SGX driver & PSW\n");
		return -1;
	}

	if (method_exists == 1) {
		set_setting("sgx-vm-method", TYPE_ID_STR, name);
		sgx_destroy_enclave(eid);
		return 0;
	}

	char methods_error[bufsize];
	memset(methods_error, '\0', bufsize);

	status = ecall_get_vm_methods_error(eid, methods_error, bufsize);
	if (status != SGX_SUCCESS) {
		pr_fail("Unable to enter enclave, check SGX driver & PSW\n");
		return -1;
	}

	(void)fprintf(stderr, methods_error);
	sgx_destroy_enclave(eid);
	return -1;
}

/*
 *  stress_sgx_vm()
 *	stress virtual memory
 */
int stress_sgx_vm(const args_t *args)
{
	uint64_t *bit_error_count = MAP_FAILED;
	uint64_t vm_hang = DEFAULT_VM_HANG;
	uint32_t restarts = 0, nomems = 0;
	size_t vm_bytes = DEFAULT_VM_BYTES;
	uint8_t *buf = NULL;
	char* vm_method;
	pid_t pid;
	const bool keep = (g_opt_flags & OPT_FLAGS_SGX_VM_KEEP);
        const size_t page_size = args->page_size;
	size_t buf_sz, retries;
	int err = 0, ret = EXIT_SUCCESS;
	int vm_flags = 0;                      /* VM mmap flags */
	int vm_madvise = -1;

	(void)get_setting("sgx-vm-hang", &vm_hang);
	(void)get_setting("sgx-vm-flags", &vm_flags);
	(void)get_setting("sgx-vm-method", &vm_method);
	(void)get_setting("sgx-vm-madvise", &vm_madvise);

	pr_dbg("%s using method '%s'\n", args->name, vm_method);

	if (!get_setting("sgx-vm-bytes", &vm_bytes)) {
		if (g_opt_flags & OPT_FLAGS_MAXIMIZE)
			vm_bytes = MAX_VM_BYTES;
		if (g_opt_flags & OPT_FLAGS_MINIMIZE)
			vm_bytes = MIN_VM_BYTES;
	}
	vm_bytes /= args->num_instances;
	if (vm_bytes < MIN_VM_BYTES)
		vm_bytes = MIN_VM_BYTES;
	buf_sz = vm_bytes & ~(page_size - 1);

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
		int no_mem_retries = 0;

		(void)setpgid(0, g_pgrp);
		stress_parent_died_alarm();

		/* Make sure this is killable by OOM killer */
		set_oom_adjustment(args->name, true);

		/* TODO Enter enclave and stress
		do {
			if (no_mem_retries >= NO_MEM_RETRIES_MAX) {
				pr_err("%s: gave up trying to mmap, no available memory\n",
					args->name);
				break;
			}
			if (!keep || (buf == NULL)) {
				if (!g_keep_stressing_flag)
					return EXIT_SUCCESS;
				buf = (uint8_t *)mmap(NULL, buf_sz,
					PROT_READ | PROT_WRITE,
					MAP_PRIVATE | MAP_ANONYMOUS |
					vm_flags, -1, 0);
				if (buf == MAP_FAILED) {
					buf = NULL;
					no_mem_retries++;
					(void)shim_usleep(100000);
					continue;	/* Try again *//*
				}
				if (vm_madvise < 0)
					(void)madvise_random(buf, buf_sz);
				else
					(void)shim_madvise(buf, buf_sz, vm_madvise);
			}

			no_mem_retries = 0;
			(void)mincore_touch_pages(buf, buf_sz);
			*bit_error_count += func(buf, buf_sz, args->counter,
						args->max_ops << VM_BOGO_SHIFT);

			if (vm_hang == 0) {
				for (;;) {
					(void)sleep(3600);
				}
			} else if (vm_hang != DEFAULT_VM_HANG) {
				(void)sleep((int)vm_hang);
			}

			if (!keep) {
				(void)madvise_random(buf, buf_sz);
				(void)munmap((void *)buf, buf_sz);
			}
		} while (keep_stressing_vm(args));
		*/

		if (keep && buf != NULL)
			(void)munmap((void *)buf, buf_sz);

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
