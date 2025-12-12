PROJECT:
  This project is about typical Producer - consumer problem. Multiple producers
  and multiple consumers. Due to an incomming event, a producer may need to add or
  or remove or update multiple DBs and/or may need to publish messages to other
  sub-system via different communication (eg. TCP/IP, Shared memory, MQ etc.)
  mechanism. For non-thread environments, a producer will need to update these
  DBs and publish messages sequentially which is time consuming.

  To parallelize this process, the producer will create work-item for each of
  DB and/or messages publish operations and put the work-items in work-queue.
  A pool of comsumers threads pick items from queue and process them. Consumer
  threads are not aware what are they processng. Processing are encapsulated
  work-item object. Each of the consumer thread can also be producer as per
  application's requirement.

IMPLEMENTATION:
   A pool of consumer threads are created at start up time. When an event occur
   main thread (producer) will create multiple work-items as per application
   need and put them in the queue and wake up consumers threads to de-queue
   and process the items.
   Work-item is nothing but different DBs and message-publish items.
   WorkItem is base class which has pure virtual function process().
   For each DBs and message there is one derived class whcih is derived from
   WorkItem. Each of Derived class has it's owm implemention of process.
   Each of Consumer thread just pick a WorkItem and call process(). So
   implemenation of WorkItem are not known to thread.

   STL thread are used to implement the above project. Two approach are
   used:
   (a) Joinable Thread Pool: Threads run in infinite loop and waiting on
       data to be available in Queue. These data are enqueued by main (or another thread from
       the thread pool) thread. main thread will wait after enqueing data. When all data
       in queue are processed, thread notify main thread that queue is empty. Then
       main will notify thread pool to terminate via atomic boolean
       variable and wait for them to join with main thread (TerminateThreadPool()). Then
       the server will exit gracefully.

    (b) Detachable Thread Pool: This approach is same as (a) except that
        main thread does not call TerminateThreadPool(). When queue is
        empty, thread pool notify main which then exits and eventually
        thread pool also exit.


BUILD:
$ g++ -v
Using built-in specs.
COLLECT_GCC=g++
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-pc-cygwin/12/lto-wrapper.exe
Target: x86_64-pc-cygwin
...
gcc version 12.4.0 (GCC)
