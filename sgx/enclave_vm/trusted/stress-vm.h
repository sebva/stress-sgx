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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ENCLAVE_VM_TRUSTED_STRESS_VM_H_
#define ENCLAVE_VM_TRUSTED_STRESS_VM_H_

#include <sys/types.h>
#include <stdbool.h>


bool* g_keep_stressing_flag;
uint64_t g_opt_flags;


/*
 *  the VM stress test has diffent methods of vm stressor
 */
typedef size_t (*stress_vm_func)(uint8_t *buf, const size_t sz,
		uint64_t *counter, const uint64_t max_ops);

typedef struct {
	const char *name;
	const stress_vm_func func;
} stress_vm_method_info_t;

extern const stress_vm_method_info_t vm_methods[];


#endif /* ENCLAVE_VM_TRUSTED_STRESS_VM_H_ */
