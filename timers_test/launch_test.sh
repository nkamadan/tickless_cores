#!/bin/bash

if [ "$(id -u)" != "0" ]; then
   echo "RUN THIS AS ROOT!!" 1>&2
   exit 1
fi

TRACING=/sys/kernel/debug/tracing/
# Make sure tracing is off for now
echo 0 > $TRACING/tracing_on
# Flush previous traces
echo > $TRACING/trace
# Record disturbance from other tasks
echo 1 > $TRACING/events/sched/sched_switch/enable
# Record disturbance from interrupts
echo 1 > $TRACING/events/irq_vectors/enable
# Now we can start tracing
echo 1 > $TRACING/tracing_on
# Run the dummy user_loop for 10 seconds on CPU 7
./timer_test &
USER_LOOP_PID=$!
sleep 10
kill $USER_LOOP_PID
# Disable tracing and save traces from CPU 7 in a file
echo 0 > $TRACING/tracing_on
cat $TRACING/per_cpu/cpu7/trace > trace.7
