#include "stress-ng.h"

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
	printf("HELLO SGX!\n");
	return EXIT_SUCCESS;
}
