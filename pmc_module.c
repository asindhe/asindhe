#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/timekeeping.h>
#include <asm/msr.h>
#include <asm/processor.h>
#include <linux/sched.h> // For current
#include <linux/sched/task.h> // For set_cpus_allowed_ptr
#include <linux/cpumask.h>

#define MSR_IA32_FIXED_CTR_CTRL 0x38d
#define MSR_IA32_PERF_GLOBAL_CTRL 0x38f
#define MSR_30A 0x30a
#define MSR_TSC 0x10
#define ITERATIONS 1000
//#define ITERATIONS 1000000

#ifdef PMC
uint64_t frequency[ITERATIONS], total_cycles=0, prev_total_cycle=0;
uint64_t frequency_a[ITERATIONS], total_cycles_a[ITERATIONS], elapse_time_a[ITERATIONS];
uint64_t elapse_time=0, dev=0;
uint64_t min=0, max =0;
uint64_t max_value=0xffffffffffff; 
int i=0, max_I=0, min_I=0, msr_dev=0;
#endif
int i=0, msr_dev=0;

// Function to read a PMC
static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
    return ((uint64_t)high << 32) | low;
}

#ifdef PMC
// Function to read the Time Stamp Counter (TSC)
static inline uint64_t read_tsc(uint32_t msr) {
    unsigned int low, high;
    asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
    return ((uint64_t)high << 32) | low;
}

void clear_counters(void) {
    uint64_t value = 0; // Event select for CPU clock cycles
//    wrmsrl_on_cpu(cpu, MSR_IA32_FIXED_CTR_CTRL, value);
 //   wrmsrl_on_cpu(cpu, MSR_IA32_PERF_GLOBAL_CTRL, value);
    wrmsrl(MSR_IA32_FIXED_CTR_CTRL, value);
    wrmsrl(MSR_IA32_PERF_GLOBAL_CTRL, value);
}

// Function to configure the PMC to count CPU clock cycles
void configure_pmc(void) {
    uint64_t value1 = 0x30; // Event select for CPU clock cycles
    uint64_t value2 = 0x200000000;
    //wrmsrl_on_cpu(cpu, MSR_IA32_FIXED_CTR_CTRL, value1);
    //wrmsrl_on_cpu(cpu, MSR_IA32_PERF_GLOBAL_CTRL, value2);
    wrmsrl(MSR_IA32_FIXED_CTR_CTRL, value1);
    wrmsrl(MSR_IA32_PERF_GLOBAL_CTRL, value2);
}
#endif

static void performance_monitor_counters (int cpu) {
    //uint64_t start_cycles[ITERATIONS], end_cycles[ITERATIONS], frq_msr[ITERATIONS], frq_774[ITERATIONS], frq_199[ITERATIONS];
    uint64_t frq_msr[ITERATIONS];
    //uint64_t start_tsc[ITERATIONS], end_tsc[ITERATIONS], halt_cycle[ITERATIONS];
    //int j, k;

    //printk(KERN_INFO "======================CPU: %d BEGIN======================================\n", cpu);
//	local_irq_disable();

    for (i = 0; i < ITERATIONS ; i++) {
        // Read the PMC and TSC before the interval
	//rdmsrl_on_cpu(cpu, MSR_TSC, &start_tsc);
	//rdmsrl_on_cpu(cpu, MSR_30A, &start_cycles)
#ifdef PMC
	if ( i % 2 == 0) {
	start_tsc[j] = read_tsc(0x10);
	start_cycles[j] = read_msr(0x30a);
	j++;
	//mid_tsc = read_tsc(0x10);
	}

        // Read the PMC and TSC after the interval
	else {
	end_cycles[k] = read_msr(0x30a);
	end_tsc[k] = read_tsc(0x10);
	k++;
	}
#endif
	//frq_199[i] = read_msr(0x199);
	//frq_774[i] = read_msr(0x774);
	frq_msr[i] = read_msr(0x198);
        udelay(1);

    }
//	local_irq_enable();
	//mdelay(50);
	//udelay(1);
	//elapse_time += (end_tsc - start_tsc - 80) * 1000 / 2100;
#ifdef PMC
    for (i = 0; i < (ITERATIONS / 2); i++) {
//	    if ( i > 0) {
//		prev_freq = frequency;
//		prev_elapse_time = elapse_time;
//		prev_total_cycle = total_cycle;
//	    }
	elapse_time_a[i] += (end_tsc[i] - start_tsc[i] - 80) * 1000 / 2100;
	elapse_time += (end_tsc[i] - start_tsc[i] - 80) * 1000 / 2100;

	if (end_cycles[i] < start_cycles[i]) {
		//need to take care for total_cycle ??
		total_cycles_a[i] += (max_value - start_cycles[i]) + end_cycles[i] + 1;
		total_cycles += (max_value - start_cycles[i]) + end_cycles[i] + 1;
		printk(KERN_INFO "Total cycle: %llu\n", total_cycles);
		printk(KERN_INFO "Rollover\n");
	}
	else{
		//need to take care for total_cycle ??
		total_cycles_a[i] += end_cycles[i] - start_cycles[i];
		total_cycles += end_cycles[i] - start_cycles[i];
	}
	if (total_cycles == 0)
		printk(KERN_INFO "total cycle ZERO\n");

	if ( (end_tsc[i] - start_tsc[i]) > (end_cycles[i] - start_cycles[i])) {
	halt_cycle[i] = (((end_tsc[i] - start_tsc[i]) - (end_cycles[i] - start_cycles[i])) * 100) /(end_tsc[i] - start_tsc[i]) * 100;
	halt_cycle[i] /=100;
	printk(KERN_INFO " end_tsc:%llu, start_tsc:%llu, delta_tsc: %llu\n", end_tsc[i], start_tsc[i], (end_tsc[i] - start_tsc[i]));
	printk(KERN_INFO " end_cycles:%llu, start_cycles:%llu, delta_cycle: %llu\n", end_cycles[i], start_cycles[i], (end_cycles[i] - start_cycles[i]));
	printk(KERN_INFO " halt_cycle:%llu%%\n", halt_cycle[i]);
	}
	else {
		halt_cycle[i] = 0;
	}
        //frequency[i] =  total_cycles * 1000000 / (ts_end.tv_nsec - ts_start.tv_nsec - 45);
        frequency_a[i] =  total_cycles_a[i] * 1000 / elapse_time_a[i];
        frequency[i] =  total_cycles * 1000 / elapse_time;
        //frequency[i] = (start_cycles - end_cycles) * 1000000 / (ts_start.tv_nsec - ts_end.tv_nsec);
	if (frequency[i] > 4040)
		printk(KERN_INFO "frequency is above range : %llu\n", frequency[i]);
	//printk(KERN_INFO "  delta_cycles:%llu\n", (end_cycles[i] - start_cycles[i]));
	//printk(KERN_INFO "  frequency:%llu\n", frequency[i]);
//	if (i > 0) {
//	if ( prev_freq > frequency ) {
//		dev = (prev_freq - frequency) * 100 / prev_freq *100;
//	else
//		dev = (frequency - prev_freq) * 100 / frequency * 100;

//	}
    }
    	/*min = frequency[0];
	for (i = 0; i < ITERATIONS >> 1 ; i++ ) {
	if (frequency[i] > max) {
		max = frequency[i];
		max_I = i;
	}
	if (frequency[i] < min) {
		min = frequency[i];
		min_I = i;
	}
	}*/
#endif
    for (i = 0; i < ITERATIONS - 1; i++ ) {
	    frq_msr[i] &= 0xFF00;
	    frq_msr[i+1] &= 0xFF00;
	    if (frq_msr[i] > frq_msr[i+1])
		msr_dev = ((frq_msr[i] - frq_msr[i+1] ) * 100) / frq_msr[i] *100;
	    else
		msr_dev = ((frq_msr[i+1] - frq_msr[i]) * 100) / frq_msr[i+1] * 100;
	    msr_dev/=100;
	    if(msr_dev > 1) {
		    printk(KERN_INFO "msr_dev: %d%%, MSR freq: 0x%X, MSR freq +1 : 0x%X\n", msr_dev, frq_msr[i], frq_msr[i+1]);
		    printk(KERN_INFO "*********************************************************\n");
	    }
    }
#ifdef PMC
	for (i = 0; i < ((ITERATIONS / 2) - 1) ; i++ ) {
	if (frequency[i] > frequency[i+1])
		dev = ((frequency[i] - frequency[i+1]) * 100 / frequency[i]) * 100;
	else
		dev = ((frequency[i+1] - frequency[i]) * 100/ frequency[i+1]) * 100;
	dev/=100;
		//printk(KERN_INFO "dev: %llu\n", dev);
    		//printk(KERN_INFO "prev: %llu, frequency: %llu\n", frequency[i], frequency[i+1]);
	if (dev > 1){
		for (j = 0; j < (ITERATIONS / 2) ; j ++) {
			printk(KERN_INFO " Cal frequency: %llu, MSR frequency: 0x%X\n", frequency[i], frq_msr[j]);
			printk(KERN_INFO "total cycle: %llu,Elapse time: %llu, start cycle: %llu, end_cycle: %llu\n", total_cycles_a[j], elapse_time_a[j], start_cycles[j], end_cycles[j]);
		}
		printk(KERN_INFO "Found dev: %d%%, CPU : %2d, Iteration:%d\n", dev, cpu, i);
		//printk(KERN_INFO "total cycle: %llu, prev cycle: %llu, Elapse time: %llu, start cycle: %llu, end_cycle: %llu\n", total_cycles, prev_total_cycle, elapse_time, start_cycles[i], end_cycles[i]);
    		printk(KERN_INFO "Found current: %llu, next frequency: %llu, frq n-1: 0x%X, frq curr from msr: 0x%X, frq next from msr: 0x%X, frq n=2: 0x%X, halt frq: %llu\n", frequency[i], frequency[i+1], frq_msr[i-1], frq_msr[i], frq_msr[i+1], frq_msr[i+2], halt_cycle[i]);
		//printk(KERN_INFO "total cycle prev: %llu, total cycle curr: %llu, total cycle next: %llu\n", total_cycles[i-1], total_cycles[i], total_cycles[i+1]);
		//printk(KERN_INFO "Elapse time prev: %llu, Elapse time curr: %llu, Elapse time next: %llu\n", elapse_time[i-1], elapse_time[i], elapse_time[i+1]);
		//printk(KERN_INFO "delta_time in nsec:%llu, start: %llu, end: %llu\n", (ts_end.tv_nsec - ts_start.tv_nsec - 45), ts_start.tv_nsec, ts_end.tv_nsec);
		//printk(KERN_INFO "delta_xtime in nsec:%llu, x_start: %llu, end: %llu\n", (x_end - x_start), x_start, x_end);
		printk(KERN_INFO "CURR: end_cycles: %llu, start_cycles: %llu, delat_cycle: %llu\n", end_cycles[i], start_cycles[i], (end_cycles[i] - start_cycles[i]));
		printk(KERN_INFO "NEXT: end_cycles: %llu, start_cycles: %llu, delat_cycle: %llu\n", end_cycles[i+1], start_cycles[i+1], (end_cycles[i+1] - start_cycles[i+1]));
		printk(KERN_INFO "CURR: end_tsc:%llu, start_tsc:%llu, deltae_time: %llu\n", end_tsc[i], start_tsc[i], (end_tsc[i] - start_tsc[i]));
		printk(KERN_INFO "NEXT: end_tsc:%llu, start_tsc:%llu, deltae_time: %llu\n", end_tsc[i+1], start_tsc[i+1], (end_tsc[i+1] - start_tsc[i+1]));
		return;
	}
        //printk(KERN_INFO "CPU: %d, Frequency: %llu, frq from msr: 0x%X, msr 0x199 : 0x%X, msr 0x774: 0x%X\n", cpu, frequency[i], frq_msr[i], frq_199[i], frq_774[i]);
        //printk(KERN_INFO "CPU: %d, Frequency: %llu, frq from msr: 0x%X, msr 0x199 : 0x%X, msr 0x774: 0x%X\n", cpu, frequency[i], frq_msr[i]);
	//printk(KERN_INFO "end_cycles: %llu, start_cycles: %llu, delat_cycle: %llu\n", end_cycles, start_cycles, (end_cycles - start_cycles));
	//printk(KERN_INFO "end_tsc:%llu, start_tsc:%llu, elapse_time: %llu\n", end_tsc, start_tsc, elapse_time);
	//printk(KERN_INFO " read msr delay: %llu\n", (mid_tsc - start_tsc));

    }
#endif
    //printk(KERN_INFO "After while dev: %llu\n", dev);
    //printk(KERN_INFO "After while prev[i]: %llu, frequency[i]: %llu\n", prev[i], frequency);
    /*dev_m = (max - min) * 100 / max * 100;
    if (dev_m > 100) {
	    printk(KERN_INFO "Found dev_m: %3d, CPU : %2d\n", dev_m, cpu);
	    printk(KERN_INFO "min: %llu, min_I: %d, max: %llu, max_I: %d\n", min, min_I, max, max_I);
    }*/

    //printk(KERN_INFO " min: %llu, min_I: %d, max: %llu, max_I: %d\n", min, min_I, max, max_I);
    //printk(KERN_INFO "======================CPU: %d END======================================\n", cpu);
}
static int __init pmc_module_init(void) {
	int cpu=6;
//	uint64_t total_start, total_end, total_elapsed;
	cpumask_t mask;
     	//printk(KERN_INFO "PMC Module Loaded\n");

//	total_start = read_tsc(0x10);
//    	for (cpu=1; cpu < 2; cpu ++) {
		cpumask_clear(&mask);
		cpumask_set_cpu(cpu, &mask);
		set_cpus_allowed_ptr(current, &mask);
    	//	clear_counters();
    //		configure_pmc();
//		udelay(100);
    		performance_monitor_counters(cpu);
 //   	}
//	total_end = read_tsc(0x10);
//	printk(KERN_INFO " Total time consumed : %llu\n", (total_end - total_start) * 1000 / 2100 );
	return 0;
}

static void __exit pmc_module_exit(void) {
    //printk(KERN_INFO "PMC Module Unloaded\n");
}

module_init(pmc_module_init);
module_exit(pmc_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ASINDHE");
MODULE_DESCRIPTION("A kernel module to measure CPU frequency[i] using PMCs");
