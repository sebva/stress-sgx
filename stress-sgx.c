#include "stress-ng.h"
#include "sgx/utils.h"
#include "sgx/enclave_enclave/untrusted/enclave_u.h"

int stress_sgx_supported(void)
{
	return 0;
}

/*
 *  stress_sgx
 *	Various SGX-related stressors
 */
int stress_sgx(const args_t *args)
{
	sgx_enclave_id_t eid = 0;
	sgx_status_t status = 0;

	/* Initialize the enclave */
	status = initialize_enclave(&eid);
	if (status != SGX_SUCCESS){
		printf("Error %d\n", status);
		return -1;
	}


	printf("Will ECALL into enclave\n");
	int ret;
	status = ecall_enclave_sample(eid, &ret);
	if (status != SGX_SUCCESS) {
		print_error_message(status);
		abort();
	}
	printf("Enclave returned %d\n", ret);

	sgx_destroy_enclave(eid);
	printf("Enclave destroyed\n");
	return EXIT_SUCCESS;
}
