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

# define TOKEN_FILENAME   "stress-sgx.token"
# define ENCLAVE_FILENAME "enclave.signed.so"

int initialize_enclave(sgx_enclave_id_t* eid);
void print_error_message(sgx_status_t ret);

#endif
