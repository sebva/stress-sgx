#include "stress-ng.h"
#include "sgx/enclave_cpu/trusted/stress-cpu.h"

/*
 *  stress_sgx
 *	Various SGX-related stressors
 */
int stress_cpu(const args_t *args)
{
	stress_cpu_method_info_t* method;
	get_setting("cpu-method", &method);
	pr_dbg("Method will be %s\n", method->name);


	pr_dbg("Will call into enclave.so\n");
	int ret;
	ret = real_stress_cpu(method->name, args->max_ops, (args->counter), &g_keep_stressing_flag, g_opt_flags);

	pr_dbg("Call returned\n");

	switch(ret) {
	case 0:
		return EXIT_SUCCESS;
	case -1:
		printf("Please set --cpu-method first\n");
	}

	return EXIT_FAILURE;
}

void stress_set_cpu_load(const char *opt) {
	int32_t cpu_load;

	cpu_load = get_int32(opt);
	check_range("cpu-load", cpu_load, 0, 100);
	set_setting("cpu-load", TYPE_ID_INT32, &cpu_load);
}

/*
 *  stress_set_cpu_load_slice()
 *	< 0   - number of iterations per busy slice
 *	= 0   - random duration between 0..0.5 seconds
 *	> 0   - milliseconds per busy slice
 */
void stress_set_cpu_load_slice(const char *opt)
{
	int32_t cpu_load_slice;

	cpu_load_slice = get_int32(opt);
	if ((cpu_load_slice < -5000) || (cpu_load_slice > 5000)) {
		(void)fprintf(stderr, "cpu-load-slice must in the range -5000 to 5000.\n");
		exit(EXIT_FAILURE);
	}
	set_setting("cpu-load-slice", TYPE_ID_INT32, &cpu_load_slice);
}

/*
 *  stress_set_cpu_method()
 *	set the default cpu stress method
 */
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
