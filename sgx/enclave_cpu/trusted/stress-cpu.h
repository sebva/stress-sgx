#ifndef _STRESS_CPU_H
#define _STRESS_CPU_H

/*
 *  the CPU stress test has different classes of cpu stressor
 */
typedef void (*stress_cpu_func)(const char *name);

typedef struct {
	const char		*name;	/* human readable form of stressor */
	const stress_cpu_func	func;	/* the cpu method function */
} stress_cpu_method_info_t;

extern const stress_cpu_method_info_t cpu_methods[];

int real_stress_cpu(const char* method_name, const uint64_t rounds, uint64_t *const counter , _Bool* keep_stressing_flag, uint64_t opt_flags);

#endif
