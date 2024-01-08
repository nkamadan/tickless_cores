#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "RUN THIS AS ROOT!!" 1>&2
   exit 1
fi
# Migrate irqs to CPU 0-6 (exclude CPU 7)
for I in $(ls /proc/irq)
do
    if [[ -d "/proc/irq/$I" ]]
    then
        echo "Affining vector $I to CPUs 0-6"
        echo 0-6 > /proc/irq/$I/smp_affinity_list
    fi
done

