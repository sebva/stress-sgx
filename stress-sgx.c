#include "stress-ng.h"
#include "sgx/utils.h"
#include "sgx/enclave_enclave/untrusted/enclave_u.h"

typedef void (*stress_cpu_func)(const char *name);
typedef struct {
	const char		*name;	/* human readable form of stressor */
	const stress_cpu_func	func;	/* the cpu method function */
} stress_cpu_method_info_t;


int stress_sgx_supported(void)
{
	return 0;
}

void ocall_pr_fail(const char* str)
{
	pr_fail(str);
}

uint64_t ocall_dummy(uint64_t param)
{
	return param + 1;
}

/*
 *  stress_sgx
 *	Various SGX-related stressors
 */
int stress_sgx(const args_t *args)
{
	stress_cpu_method_info_t* method;
	get_setting("cpu-method", &method);
	pr_dbg("Method will be %s\n", method->name);


	sgx_enclave_id_t eid = 0;
	sgx_status_t status = 0;

	/* Initialize the enclave */
	status = initialize_enclave(&eid);
	if (status != SGX_SUCCESS){
		printf("Error %d\n", status);
		return -1;
	}


	pr_dbg("Will ECALL into enclave\n");
	int ret;
	status = ecall_stress_cpu(eid, &ret, method->name, args->max_ops, (args->counter), &g_keep_stressing_flag, g_opt_flags);
	if (status != SGX_SUCCESS) {
		print_error_message(status);
		abort();
	}

	sgx_destroy_enclave(eid);
	pr_dbg("Enclave destroyed\n");

	switch(ret) {
	case 0:
		return EXIT_SUCCESS;
	case -1:
		printf("Please set --cpu-method first\n");
	}

	return EXIT_FAILURE;
}
