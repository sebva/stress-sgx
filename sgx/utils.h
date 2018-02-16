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
#ifndef __SGX_UTILS
#define __SGX_UTILS

#include <sgx_urts.h>

#ifndef TRUE
# define TRUE 1
#endif

#ifndef FALSE
# define FALSE 0
#endif

#ifndef NULL
# define NULL 0
#endif

# define TOKEN_CPU_FILENAME   "stress-sgx-cpu.token"
# define ENCLAVE_CPU_FILENAME "enclave_cpu.signed.so"
# define TOKEN_VM_FILENAME   "stress-sgx-vm.token"
# define ENCLAVE_VM_FILENAME "enclave_vm.signed.so"

int initialize_enclave(sgx_enclave_id_t* eid, char* enclave_file, char* enclave_token);
void print_error_message(sgx_status_t ret);

#endif
