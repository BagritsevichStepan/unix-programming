# Unix Programming
Repository contains unix programming tasks for network programming, ipc and multithreading.

# Tasks
* [Parallel Programming](#parallel)
    1. [OpenMP. Intel TBB. MPI](#openmp)
    2. [POSIX Threads](#posix)
* [Network Programming](#network)
    1. [Libevent. Libev. Libuv. Boost.Asio](#async)
    2. [Sockets. Multiplexing](#sockets)
* [Unix IPC](#ipc)
    1. [Unix Processes. Channels. Signals](#fork)
    2. [Unix Message Queues. Semaphores](#msg)
  
## <a name="parallel"></a>Parallel Programming
### <a name="openmp"></a>OpenMP. Intel TBB. MPI
1. Parallelize finding the [determinant](https://en.wikipedia.org/wiki/Determinant) of matrix
2. Implement parallel [prefix sum](https://en.wikipedia.org/wiki/Prefix_sum) algorithm

The solutions are in the folder `parallel_programming`.

### <a name="posix"></a>POSIX Threads
1. Create three threads. The first thread must wait for the end of the third thread, and the second must cancel the third. File: `pthread_canceled.cpp`
2. Implement producer and consumer using `pthread_rwlock_t` and `pthread_mutex_lock`. File: `pthread_mutex.cpp`
3. Implement producer and consumer using `pthread_cond_t`. File: `wait_pthread`

The solutions are in the folder `posix_threads`.

## <a name="network"></a>Network Programming
### <a name="async"></a>Libevent. Libev. Libuv. Boost.Asio
Implement asynchronous echo-server using each of these technologies: [`libevent`](https://libevent.org), [`libev`](http://software.schmorp.de/pkg/libev.html), [`libuv`](https://libuv.org), [`Boost.Asio`](https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio.html)

The solutions are in the folder `asynchronous_network`.

### <a name="sockets"></a>Sockets. Multiplexing
1. Implement echo-server and client using each of these technologies:
    - simple TCP approach
    - multiplexing using `select`
    - multiplexing using `poll`
    - multiplexing using `epoll`
    - multiplexing using `kqueue`
2. Implement asynchoronous chat-server using `epoll`
3. Implement the display of IP addresses for the given host on the command line in `showip.c`

The solutions are in the folder `sockets_multiplexing`.

## <a name="ipc"></a>Unix IPC
### <a name="fork"></a>Unix Processes. Channels. Signals
1. The user enters several bash commands into the command line. For example:
   
   `who | sort | uniq -c | sort -nk1`
   
   We need to read the data from `STDINT` and output result to `STDOUT`. Each command must be executed in separate unix process. To transfer data between processes, use `pipe`
3. Implement inter-process communication using `pipe`, named pipe `fifo channel` and `socketpair`
4. Implement unkillable proccess. The process must ignore the signals `SIGTERM` and `SIGINT`
5. Create child process that waits for and processes the `SIGTERM` signal from the parent process

The solutions are in the folder `processes_channels_signals`.

### <a name="msg"></a>Unix Message Queues. Semaphores
1. Implement process communication using unix message queues. Сreate two solutions: one with POSIX `mq_open`, the second with SysV `msgget` message queue
2. Implement process synchronization using unix semaphores. Сreate two solutions: one with POSIX `sem_open`, the second with SysV `semget` semaphore

The solutions are in the folder `unix_ipc`.
