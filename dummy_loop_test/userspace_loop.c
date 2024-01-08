#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
int main(void)
{
    // Move the current task to the isolated cgroup (bind to CPU 7)
    int fd = open("/sys/fs/cgroup/cpuset/isolated/cgroup.procs", O_WRONLY);
    if (fd < 0) {
        perror("Can't open cpuset file...\n");
        return 0;
    }
   
    write(fd, "0\n", 2); //writing 0\n to cgroup.procs --> movethe process that performs this write operation to the isolated cgroup.
    close(fd);
   
    // Run an endless dummy loop until the launcher kills us
    while (1)
        ;
   
    return 0;
}
