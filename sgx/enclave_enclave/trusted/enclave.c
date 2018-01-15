#include "enclave_t.h"  /* print_string */
#include "stress-cpu.c"

/*
 *  keep_stressing()
 *	returns true if we can keep on running a stressor
 */
bool HOT OPTIMIZE3 keep_stressing(const uint64_t rounds, uint64_t counter)
{
	return (LIKELY(*g_keep_stressing_flag) &&
				LIKELY(!rounds || (counter < rounds)));
}

void run_stressor(const stress_cpu_method_info_t* info, const uint64_t rounds) {
	uint64_t counter = 0;

	do {
		(info->func)("stress-sgx");
		counter++;
	} while(keep_stressing(rounds, counter));
}

int ecall_stress_cpu(const char* method_name, const uint64_t rounds, bool* keep_stressing_flag, uint64_t opt_flags)
{
	stress_cpu_method_info_t const *info;

	g_opt_flags = opt_flags;
	g_keep_stressing_flag = keep_stressing_flag;

	if (g_keep_stressing_flag == 0) {
		return -1;
	}

	for (info = cpu_methods; info->func; info++) {
		if (!strcmp(info->name, method_name)) {
			run_stressor(info, rounds);
			return 0;
		}
	}

	return -1;
}
