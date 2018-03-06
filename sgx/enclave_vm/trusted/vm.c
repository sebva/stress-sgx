/*
 * Stress-SGX: Load and stress your enclaves for fun and profit
 * Copyright (C) 2018 Sébastien Vaucher
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

#include "vm_t.h"  /* print_string */
#include "stress-vm.h"
#include "companion.h"
#include <stdio.h>

#define NO_MEM_RETRIES_MAX	(100)

/*
 *  keep_stressing()
 *	returns true if we can keep on running a stressor
 */
static bool HOT OPTIMIZE3 keep_stressing_vm(const uint64_t rounds,
		uint64_t * const counter) {
	return (LIKELY((*g_keep_stressing_flag))
			&& LIKELY(!rounds || ((*counter >> VM_BOGO_SHIFT) < rounds)));
}

int ecall_vm_method_exists(const char* method_name) {
	stress_vm_method_info_t const *info;
	for (info = vm_methods; info->func; info++) {
		if (strcmp(info->name, method_name) == 0) {
			return 1;
		}
	}
	return 0;
}

void ecall_get_vm_methods_error(char* out_methods, int length) {
	stress_vm_method_info_t const *info;
	int counter = 0;

	if (sgx_is_outside_enclave(out_methods, length)) {
		counter += snprintf(out_methods, length,
				"sgx-vm-method must be one of:");
		for (info = vm_methods; info->func; info++) {
			counter += snprintf(out_methods + counter, length - counter, " %s",
					info->name);
		}
		(void) snprintf(out_methods + counter, length - counter, "\n");
	}
}

int ecall_stress_vm(size_t vm_bytes, const char* method_name,
		const uint64_t rounds, uint64_t * const counter,
		bool* keep_stressing_flag, uint64_t opt_flags,
		uint64_t *bit_error_count, size_t page_size, uint64_t vm_hang) {
	return real_stress_vm(vm_bytes, method_name, rounds, counter, keep_stressing_flag,
			opt_flags, bit_error_count, page_size, vm_hang,
			&malloc, &free);
}

int real_stress_vm(size_t vm_bytes, const char* method_name,
		const uint64_t rounds, uint64_t * const counter,
		bool* keep_stressing_flag, uint64_t opt_flags,
		uint64_t *bit_error_count, size_t page_size, uint64_t vm_hang,
		void* (*allocate_function)(size_t), void (*deallocate_function)(void*)) {
	uint8_t *buf = NULL;
	int no_mem_retries = 0;
	size_t buf_sz;
	const bool keep = (g_opt_flags & OPT_FLAGS_SGX_VM_KEEP);
	buf_sz = vm_bytes & ~(page_size - 1);
	const stress_vm_method_info_t *info = &vm_methods[0];
	stress_vm_func func;

	g_opt_flags = opt_flags;
	g_keep_stressing_flag = keep_stressing_flag;

	for (info = vm_methods; info->func; info++) {
		if (!strcmp(info->name, method_name)) {
			func = info->func;
			break;
		}
	}

	do {
		if (no_mem_retries >= NO_MEM_RETRIES_MAX) {
			ocall_pr_err("stress-sgx-vm: gave up trying to allocate trusted memory, no available memory\n");
			break;
		}
		if (!keep || (buf == NULL)) {
			if (!g_keep_stressing_flag)
				return EXIT_SUCCESS;
			buf = (uint8_t *) ((*allocate_function)(buf_sz));
			if (buf == NULL) {
				no_mem_retries++;
				int ret;
				(void) ocall_shim_usleep(&ret, 100000);
				continue; /* Try again */
			}
		}

		no_mem_retries = 0;
		(void) mincore_touch_pages(buf, buf_sz);
		*bit_error_count += func(buf, buf_sz, counter, rounds << VM_BOGO_SHIFT);

		if (vm_hang == 0) {
			for (;;) {
				(void) ocall_sleep(3600);
			}
		} else if (vm_hang != DEFAULT_VM_HANG) {
			(void) ocall_sleep((int) vm_hang);
		}

		if (!keep) {
			(*deallocate_function)(buf);
		}
	} while (keep_stressing_vm(rounds, counter));

	if (keep && buf != NULL)
		(*deallocate_function)(buf);
}

int real_vm_method_exists(const char* method_name) {
	return ecall_vm_method_exists(method_name);
}

void real_get_vm_methods_error(char* out_methods, int length) {
	return ecall_get_vm_methods_error(out_methods, length);
}
