#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define CGT_GTD_RDTSC 1 //0=clock_gettime ...
#define DEBUG_ISOLATION 0 //for debugging, use taskset to pin the process to a non-isolated core.
#define TIMESPEC_NSEC(ts) ((ts)->tv_sec * 1e9 + (ts)->tv_nsec)

static inline __attribute__ ((always_inline))
uint64_t clock_gettime_test()
{
	struct timespec now_ts;
	clock_gettime(CLOCK_MONOTONIC, &now_ts);
	return TIMESPEC_NSEC(&now_ts);
}

static inline __attribute__ ((always_inline))
uint64_t gettimeofday_test()
{
    struct timeval now_tv;
    gettimeofday(&now_tv, NULL);
    return (uint64_t)now_tv.tv_sec * 1000000 + now_tv.tv_usec;
}

static inline __attribute__((always_inline))
uint64_t rdtsc() {
    uint64_t a, d;
    asm volatile ("xor %%rax, %%rax\n" "cpuid"::: "rax", "rbx", "rcx", "rdx");
    asm volatile ("rdtscp" : "=a" (a), "=d" (d) : : "rcx");
    a = (d << 32) | a;
    return a;
}

int main(void)
{
    #if DEBUG_ISOLATION == 0
    // Move the current task to the isolated cgroup (bind to CPU 7)
    int fd = open("/sys/fs/cgroup/cpuset/isolated/cgroup.procs", O_WRONLY);
    if (fd < 0) {
        perror("Can't open cpuset file...\n");
        return 0;
    }
   
    write(fd, "0\n", 2); //writing 0\n to cgroup.procs --> movethe process that performs this write operation to the isolated cgroup.
    close(fd);
    #endif

    volatile uint64_t * lat = (uint64_t*)malloc(sizeof(uint64_t)*1);   
    // Run an endless dummy loop until the launcher kills us
    while (1){
	#if CGT_GTD_RDTSC > 2
	//printf("timer testing\n");    
	volatile uint64_t temp = 0;
	uint64_t start,end;

	#if CGT_GTD_RDTSC == 0
        start = clock_gettime_test();
        #elif CGT_GTD_RDTSC == 1
        start = gettimeofday_test();
        #elif CGT_GTD_RDTSC == 2
        start = rdtsc();
        #endif

	for(int x=0;x<10000;x++){
		temp++;
	}

	#if CGT_GTD_RDTSC == 0
        end = clock_gettime_test();
        #elif CGT_GTD_RDTSC == 1
        end = gettimeofday_test();
        #elif CGT_GTD_RDTSC == 2
        end = rdtsc();
        #endif

   	lat[0] = end-start;
	printf("lat: %lld\n",lat[0]);
	#endif
	//printf("dummy loop testing\n");
    }
   
    return 0;
}
