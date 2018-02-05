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
 *  stress_set_sgx_method()
 *	set the default sgx stress method
 */
int stress_set_sgx_method(const char *name)
{
	const int bufsize = 2000;
	int method_exists = 0;
	sgx_enclave_id_t eid = 0;
	sgx_status_t status;

	/* Initialize the enclave */
	status = initialize_enclave(&eid);
	if (status != SGX_SUCCESS) {
		printf("Error %d\n", status);
		return -1;
	}

	status = ecall_method_exists(eid, &method_exists, name);
	if (status != SGX_SUCCESS) {
		pr_fail("Unable to enter enclave, check SGX driver & PSW\n");
		return -1;
	}

	if (method_exists == 1) {
		set_setting("sgx-method", TYPE_ID_STR, name);
		sgx_destroy_enclave(eid);
		return 0;
	}

	char methods_error[bufsize];
	memset(methods_error, '\0', bufsize);

	status = ecall_get_methods_error(eid, methods_error, bufsize);
	if (status != SGX_SUCCESS) {
		pr_fail("Unable to enter enclave, check SGX driver & PSW\n");
		return -1;
	}

	(void)fprintf(stderr, methods_error);
	sgx_destroy_enclave(eid);
	return -1;
}

/*
 *  stress_sgx
 *	Various SGX-related stressors
 */
int stress_sgx(const args_t *args)
{
	char* method;
	get_setting("sgx-method", &method);
	pr_dbg("Method will be %s\n", method);

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
	status = ecall_stress_cpu(eid, &ret, method, args->max_ops, (args->counter), &g_keep_stressing_flag, g_opt_flags);
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
