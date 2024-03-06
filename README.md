# Generic Thread Pool

A generic thread pool that is designed for the purpose of traversing a graph and calculating the sum of its nodes' values. 
It traverses the graph in a parallelized manner usign the POSIX threads APIs provided by the library pthreads. Upon thread pool creation, N threads are created and persistently poll the task queue for assigned tasks, maintaining continuous operation without the need for thread destruction throughout their lifetime. 
