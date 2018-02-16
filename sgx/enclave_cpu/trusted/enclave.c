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
#include "enclave_t.h"  /* print_string */
#include "stress-cpu.c"
#include <stdio.h>

/*
 *  keep_stressing()
 *	returns true if we can keep on running a stressor
 */
bool HOT OPTIMIZE3 keep_stressing(const uint64_t rounds, uint64_t *const counter)
{
	return (LIKELY(*g_keep_stressing_flag) &&
				LIKELY(!rounds || ((*counter) < rounds)));
}

void run_stressor(const stress_cpu_method_info_t* info, const uint64_t rounds, uint64_t *const counter) {
	do {
		(info->func)("stress-sgx");
		(*counter)++;
	} while(keep_stressing(rounds, counter));
}

int ecall_cpu_method_exists(const char* method_name)
{
	stress_cpu_method_info_t const *info;
	for (info = cpu_methods; info->func; info++) {
		if (strcmp(info->name, method_name) == 0) {
			return 1;
		}
	}
	return 0;
}

void ecall_get_cpu_methods_error(char* out_methods, int length)
{
	stress_cpu_method_info_t const *info;
	int counter = 0;

	if (sgx_is_outside_enclave(out_methods, length)) {
		counter += snprintf(out_methods, length, "sgx-method must be one of:");
		for (info = cpu_methods; info->func; info++) {
			counter += snprintf(out_methods + counter, length - counter, " %s", info->name);
		}
		(void)snprintf(out_methods + counter, length - counter, "\n");
	}
}

int ecall_stress_cpu(const char* method_name, const uint64_t rounds,
		uint64_t * const counter, bool* keep_stressing_flag, uint64_t opt_flags) {
	stress_cpu_method_info_t const *info;

	g_opt_flags = opt_flags;
	g_keep_stressing_flag = keep_stressing_flag;

	if (g_keep_stressing_flag == 0) {
		return -1;
	}

	for (info = cpu_methods; info->func; info++) {
		if (!strcmp(info->name, method_name)) {
			run_stressor(info, rounds, counter);
			return 0;
		}
	}

	return -1;
}
