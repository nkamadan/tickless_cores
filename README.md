# Tickless Cores/Kernel

## Problem

* If the nanoseconds are important in your program, if the OS jitter bothers you, or if you encounter unexpected huge spikes like in the image below while benchmarking, you might consider running your program in an adaptive ticks CPU. 
![Huge Spikes in Memory Access Latencies (Multiple different rows in the same bank)](https://github.com/nkamadan/tickless_cores/blob/main/images/spikes.png)

* The figure 2 shows how the timer ticks look like while running the memory access latency benchmark (from kernel tracing interface). These timer ticks' behaviour changes in "IDLE" CPUs to make them more energy efficient. So, this image does not need to be consistent over different systems/kernels/programs etc. 

![Timer Ticks on CPU 2 That the Benchmark is Run](https://github.com/nkamadan/tickless_cores/blob/main/images/timer_ticks.png)
 
* This line is from the trace collected while running the benchmark pinned on a specific core: 

    <...>-4523    [007] d..2.  5941.572756: sched_switch: prev_comm=tester prev_pid=4523 prev_prio=100 prev_state=R+ ==> next_comm=kworker/7:1H next_pid=522 next_prio=100

* In a non-adaptive ticks CPU trace, it would be filled with a lot of jitter from OS such as ^^ for its bookkeeping, stat collection and such..  

## Solution

1. We need to check three kernel configuration parameters: CONFIG_NO_HZ_FULL=y, CONFIG_CPUSETS=y, and CONFIG_TRACING=y. In version 6.3.1 case, the first one is not set by default, so the kernel needs a custom compile with that option..
2. For the kernel boot parameters, must specify the nohz_full= parameter according to which CPUs wanted to isolate. For example: "nohz_full=4-7" isolates CPUs 4, 5, 6, and 7.

* Important note: When adding this boot parameter, the kernel removes any possible disturbances from these cores. If you try to pin more than one task of the same priority (prio) to the same core, a switch between these two will be necessary, hence a scheduler is required. The scheduler requires this timer tick for bookkeeping. Or, if you are running only a single task but using POSIX CPU timers in that program, these timers are dependent on these ticks. Similarly, if your task uses performance (perf) events, these events are also dependent on these ticks.

* Interesting note: In the past (presumably in the early days of nofull_hz), a 1 Hz tick (one interrupt per second) was left for handling scheduler statistics and such. This used to run on the isolated CPUs, but now they have solved this problem, and the ticks for scheduler statistics can be offloaded to other CPUs using unbound workqueues. Thus, nowadays, we can achieve a 100% tickless CPU (not trivially).

* Tasks handled with this boot parameter: Unbound timer callbacks are sent to CPUs outside of nohz_full (if they are not pinned). Unbound kernel workqueues and kernel threads are similarly assigned to CPUs outside of nohz_full (if they are not pinned). RCU (Read-Copy Update) operations are also offloaded to CPUs outside of nohz_full. Additionally, there is no need to specify the "rcu_nocbs=" parameter separately because the nohz_full parameter automatically handles this. CPU time accounting (accounting for how much time a process spends in kernel space vs. user space) is done not with ticks but with operations that are more overhead-intensive at context switches but offer higher precision.

* You need to pin each task you want to isolate to an isolated CPU. If you pin more than one task you want to isolate to the same isolated CPU, the earlier mentioned scheduler issue arises. Because the scheduler needs to operate, timer ticks return!

* The sched_setaffinity() API, the taskset tool, or interfaces like cpusets can be used to pin a task that is to be isolated to an isolated CPU.

3. Hardware interrupts do not have an affinity (except for timer and some specific interrupts). Therefore, they can jump to our isolated CPU. The nohz_full boot time parameter does not resolve this, so we need to handle it ourselves as a separate step. We must clear all interrupts from our isolated CPUs using /proc/irq/$IRQ/smp_affinity.

## Results

* When you succesfully isolate a core, the trace starting from the beginning of the program and ending at the end of the program must look like this in the ideal case:
      

      <idle>-0       [007] d..2.   250.179248: sched_switch: prev_comm=swapper/7 prev_pid=0 prev_prio=120 prev_state=R ==> next_comm=timer_test next_pid=2456 next_prio=120
      timer_test-2456    [007] d.h..   260.149678: reschedule_entry: vector=253
      timer_test-2456    [007] d.h..   260.149678: reschedule_exit: vector=253
