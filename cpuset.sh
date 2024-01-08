#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "RUN THIS AS ROOT!!" 1>&2
   exit 1
fi
cd /sys/fs/cgroup/cpuset
mkdir housekeeping
mkdir isolated
echo 0-6 > housekeeping/cpuset.cpus
echo 0 > housekeeping/cpuset.mems
echo 7 > isolated/cpuset.cpus
echo 0 > isolated/cpuset.mems
echo 0 > cpuset.sched_load_balance
echo 0 > isolated/cpuset.sched_load_balance
while read P
do
  echo $P > housekeeping/cgroup.procs
done < cgroup.procs
echo "Configuration completed. All tasks moved to housekeeping cpuset, and CPU 7 is isolated."
