#include "enclave_t.h"  /* print_string */
#include "stress-cpu.c"

/*
 *  keep_stressing()
 *	returns true if we can keep on running a stressor
 */
bool HOT OPTIMIZE3 keep_stressing(const uint64_t rounds, uint64_t* counter)
{
	return LIKELY(!rounds || (*counter < rounds));
}

void run_stressor(const stress_cpu_method_info_t* info, const uint64_t rounds) {
	uint64_t counter = 0;

	do {
		(info->func)("stress-sgx");
		counter++;
	} while(keep_stressing(rounds, &counter));
}

int ecall_stress_cpu(const char* method_name, const uint64_t rounds)
{
	stress_cpu_method_info_t const *info;

	for (info = cpu_methods; info->func; info++) {
		if (!strcmp(info->name, method_name)) {
			run_stressor(info, rounds);
			return 0;
		}
	}

	return -1;
}
