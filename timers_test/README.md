* I have observed inconsistent behaviour while testing POSIX timers like gettimeofday() and clock_gettime(). Sometimes trace is pure clean and sometimes it is just filled with a kernel thread.. 
* I believe it is safer to follow what [ https://www.kernel.org/doc/Documentation/timers/NO_HZ.txt ] documents: "POSIX CPU timers prevent CPUs from entering adaptive-tick mode. Real-time applications needing to take actions based on CPU time consumption need to use other means of doing so."
* However, the document also says: "At least one non-adaptive-tick CPU must remain
online to handle timekeeping tasks in order to ensure that system
calls like gettimeofday() returns accurate values on adaptive-tick CPUs."
* I observed using POSIX timers definitely triggers somethings sometimes but it is highly possible to get a pure clean trace with them as well (so maybe the second statement applies --> non-adaptive CPUs were able to take role in giving accurate results to my isolated core?). 
* TL;DR -- Avoid using POSIX timers as much as possible..
