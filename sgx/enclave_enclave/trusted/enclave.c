#include "enclave_t.h"  /* print_string */
#include "stress-cpu.c"

int ecall_enclave_sample()
{
	return 42;
}

/*
// stressor args
typedef struct {
	uint64_t *const counter;	// stressor counter
	const char *name;		// stressor name
	const uint64_t max_ops;		// max number of bogo ops
	const uint32_t instance;	// stressor instance #
	const uint32_t num_instances;	// number of instances
	pid_t pid;			// stressor pid
	pid_t ppid;			// stressor ppid
	size_t page_size;		// page size
} args_t;
//*/

/*
 *  stress_set_cpu_method()
 *	set the default cpu stress method
/*
int stress_set_cpu_method(const char *name)
{
	stress_cpu_method_info_t const *info;

	for (info = cpu_methods; info->func; info++) {
		if (!strcmp(info->name, name)) {
			set_setting("cpu-method", TYPE_ID_UINTPTR_T, &info);
			return 0;
		}
	}

	(void)fprintf(stderr, "cpu-method must be one of:");
	for (info = cpu_methods; info->func; info++) {
		(void)fprintf(stderr, " %s", info->name);
	}
	(void)fprintf(stderr, "\n");

	return -1;
}
//*/
