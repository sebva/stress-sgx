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
#include <stdio.h>

int ecall_vm_method_exists(const char* method_name)
{
	stress_vm_method_info_t const *info;
	for (info = vm_methods; info->func; info++) {
		if (strcmp(info->name, method_name) == 0) {
			return 1;
		}
	}
	return 0;
}

void ecall_get_vm_methods_error(char* out_methods, int length)
{
	stress_vm_method_info_t const *info;
	int counter = 0;

	if (sgx_is_outside_enclave(out_methods, length)) {
		counter += snprintf(out_methods, length, "sgx-vm-method must be one of:");
		for (info = vm_methods; info->func; info++) {
			counter += snprintf(out_methods + counter, length - counter, " %s", info->name);
		}
		(void)snprintf(out_methods + counter, length - counter, "\n");
	}
}
