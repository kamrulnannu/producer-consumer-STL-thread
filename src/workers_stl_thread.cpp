#include <string.h>
#include <errno.h>
#include "workers_stl_thread.h"

using namespace std;

static Workers pool(5);

Workers & GetThreadPool()
{
    return pool;
}

Workers::Workers(int NumThread) :
    m_NumWorkers(NumThread), m_Running(true)
{
}

void Workers::SpawnThread()
{
    // Spawn thread

    //m_Threads.resize(m_NumWorkers);  // (1)

    for (auto i = 0; (i < m_NumWorkers); ++i)
    {

        //m_Threads.push_back(new std::thread(thread_start_func, this)); OK (2)
        //m_Threads.push_back(new std::thread(&Workers::ThreadStart, this)); OK (3)

        // Create thread and start executing ThreadStart of this thread
        // object
        m_Threads.emplace_back(&Workers::ThreadStart, this);   // (4)

        /*
         * (1) is needed for the folowing line.
         * copy-assignment is deleted from thread class, so following line
         * uses move-assignment.
         */
        //m_Threads[i] = std::thread(&Workers::ThreadStart, this); OK (5)

        /*
         * Detaches the new thread from current (i.e. calling) thread and allowing both
         * to execute independently from each other. Both threads continue without
         * blocking nor synchronizing (e.g. using join()) in any way. Note that when either
         * one ends execution, its resources are released.
         *
         * After a call to detach(), the thread object becomes non-joinable and can be
         * destroyed safely.
         *
         * If we use this approach (detaching new thread from calling
         * thread), we don't have to call TerminateThreadPool(), because
         * detached threads are not join-able. Detached thread will terminate gracefully
         * when main() returns or exits.
         *
         * If main() exits/returns while detached threads are in the middle of execution,
         * incorrect results will be produced. To correct this situation, synchronization is
         * necessary between detached threads and main thread via mutex and condition_variable.
         *
         * For this example project, we are synchonizing worker threads
         * with main thread using a mutex and condition_variable. main
         * thread will wait in Workers::Wait() until all items in m_WorkItems are consumed
         * by workers. Worker thread will notify main when all items are
         * consumed in queue. So, we'll not have incorrect result.
         */
        //std::thread(&Workers::ThreadStart, this).detach(); OK (6)
    }
}


Workers::~Workers()
{
#if 0
    OK if we use line (2) and (3) in SpawnThread() to create threads

    for (auto th : m_Threads)
    {
        delete th;
    }
#endif
}

void Workers::EnqueueWorkItem(WorkItem *item)
{
    //std::lock_guard<std::mutex> lg(m_Mutex);
    std::lock_guard<std::recursive_mutex> lg(m_Mutex);

    m_WorkItems.push(item);

    m_WorkCondVar.notify_one();  // Wake up one thread, efficient
    //m_WorkCondVar.notify_all();   // Wake up all threads, expensive
}

void * thread_start_func_cpp(void *arg)
{
    if (arg == nullptr)
    {
        cerr << "thread_start_func_cpp: NULL ARG\n";
        exit(1);
    }

    //Workers * threads = (Workers *)arg;
    Workers * threads = static_cast<Workers *>(arg);
    Workers &threadRef = *threads;

    //std::unique_lock<std::mutex> ulk(threadRef.m_Mutex, std::defer_lock);
    std::unique_lock<std::recursive_mutex> ulk(threadRef.m_Mutex, std::defer_lock);

    while (threadRef.m_Running)
    {
        ulk.lock();
        while (threadRef.m_Running && std::empty(threadRef.m_WorkItems))
        {
            /*
             * The following call will release m_Mutex atomically and (Execution Susupended and)
             * wait for notify_one() or notify_all() by producer thread via (1) EnqueueWorkItem() when
             * an item is pushed into work-Q or (2) TerminateThreadPool() when no more data will be
             * available (and server is about to exit) and this method will set m_Running
             * flag to false so that threads exit gracefully.
             */
            threadRef.m_WorkCondVar.wait(ulk);

            /*
             * Waiting consumer thread waked-up by producer thread via
             * EqueueWorkItem() or TerminateThreadPool(), as data is in Q to process or
             * server is done respectively.. As part of this wake-up, it will re-get the
             * lock (m_Mutex) within wait() and will be out of this loop if this data is not
             * consumed by another consumer thread.
             *
             * NOTE: We are using loop to check empty-Q, because when this
             * thread woke-up, another thread(s) may have woke-up earlier and consumed the
             * Enqueued data and emptyed the Q.
             */
        }

        /*
         * There are items in Q and we got lock(m_Mutex) either via
         * ulk.lock() or within m_WorkCondVar.wait() within comsumer after producer issues
         * notify_one() or notify_all() upon enqueuing data or signalling
         * that server is about to exit..
         */

        if (!threadRef.m_WorkItems.empty())
        {
            /*
             * A thread is wake up by EnqueueWorkItem()
             */
            WorkItem * item = threadRef.m_WorkItems.front();
            threadRef.m_WorkItems.pop();

            ulk.unlock();   // (7)

            if (item)
            {
                item->process();

                delete item;
            }
        }
        else
        {
            /*
             * A thread is waked up by TerminateThreadPool()
             */
            ulk.unlock();  // (8)
        }

        /*
         * Now I'll get lock again to check if Q is empty.
         * If Q is empty, it will broadcast a signal to return to caller of
         * Wait().
         */
        //std::lock_guard<std::mutex> lkg(threadRef.m_Mutex);  // (9)
        std::lock_guard<std::recursive_mutex> lkg(threadRef.m_Mutex);  // (9)
        if (threadRef.m_WorkItems.empty())
        {
            // Broadcast to wake-up from Workers::Wait()
            threadRef.m_WaitCondVar.notify_one();
        }

        // ulk.unlock();    // (10)
    } // while(threadRef.m_Running)

    //cout<<"----Exiting thread: "<<std::this_thread::get_id()<<endl;
    return nullptr;
}

void Workers::ThreadStart()
{
    //std::unique_lock<std::mutex> ulk(m_Mutex, std::defer_lock);
    std::unique_lock<std::recursive_mutex> ulk(m_Mutex, std::defer_lock);

    while (m_Running)
    {
        ulk.lock();

        while (m_Running && std::empty(m_WorkItems))
        {
            /*
             * The following call will release m_Mutex atomically and (Execution Susupended and)
             * wait for notify_one() or notify_all() by producer thread via (1) EnqueueWorkItem() when
             * an item is pushed into work-Q or (2) TerminateThreadPool() when no more data will be
             * available (and server is about to exit) and this method will set m_Running
             * flag to false so that threads exit gracefully.
             */
            m_WorkCondVar.wait(ulk);

            /*
             * Waiting consumer thread waked-up by producer thread via
             * EqueueWorkItem() or TerminateThreadPool(), as data is in Q to process or
             * server is done respectively.. As part of this wake-up, it will re-get the
             * lock (m_Mutex) within wait() and will be out of this loop if this data is not
             * consumed by another consumer thread.
             *
             * NOTE: We are using loop to check empty-Q, because when this
             * thread woke-up, another thread(s) may have woke-up earlier and consumed the
             * Enqueued data and emptyed the Q.
             */
        }

        /*
         * There are items in Q and we got lock(m_Mutex) either via
         * ulk.lock() or within m_WorkCondVar.wait() within comsumer after producer issues
         * notify_one() or notify_all() upon enqueuing data or signalling
         * that server is about to exit..
         */
        if (!m_WorkItems.empty()) // Redundant
        {
            /*
             * A thread is wake up by EnqueueWorkItem()
             */
            WorkItem * item = m_WorkItems.front();
            m_WorkItems.pop();

            /*
             * Why do we need to unlock here?
             * 
             * Some item->process() (Line# 7a) may need to enqueue data which requires
             * to get lock. Without line (7), there will be dead-lock when
             * try to enqueue via process().
             *
             * But if we use std::recursive_mutex instead of std::mutex, we
             * don't need line (7), (8), (9), but we need line (10).
             */
            //ulk.unlock();  // (7)

            if (item)
            {
                item->process(); //(7a)
                delete item;
            }
        }
        else
        {
            /*
             * A thread is waked up by TerminateThreadPool()
             */
            //ulk.unlock();   // (8)
        }

        /*
         * Now I'll get lock again to check if Q is empty.
         * If Q is empty, it will broadcast a signal to return to caller of
         * Wait().
         */
        //std::lock_guard<std::mutex> lkg(m_Mutex);  // (9)
        //std::lock_guard<std::recursive_mutex> lkg(m_Mutex);  // (9)
        if (std::empty(m_WorkItems))
        {
            m_WaitCondVar.notify_one();
        }

        ulk.unlock();  // (10)
    } // while(m_Running)
}

void Workers::Wait()
{
    /*
     * Wait until all work items in Q consumed
     */
    //std::unique_lock<std::mutex> ulk(m_Mutex);
    std::unique_lock<std::recursive_mutex> ulk(m_Mutex);

    /*
     * Here means, we got lock
     */

    while (!std::empty(m_WorkItems))
    {
        /*
         * The following call will release m_Mutex atomically
         * and wait for notify_one() or notify_all() from another thread (consumer) when
         * when the Q becomes empty.
         */
         m_WaitCondVar.wait(ulk);

        /*
         * Here means, broadasted/signaled by a consumer thread from ThreadStart()
         * because all items in Q are processed.
         * We got lock within wait() upon bradcasted/signalled/notification
         * and we have to retrun from this Wait() to caller
         * who was waiting on finishing all items in Q.
         */
    }
}

void Workers::TerminateThreadPool()
{
    /*
     * This method should be called when server is done (i.e. after
     * Workers::Wait() returns).
     *
     * This method will ensure that the threads and the process terminate
     * gracefully.
     *
     * After processing all the data in Queue, threads are waiting on
     * m_WorkCondVar for more data to be available.
     *
     * Since we know that no more data will be available, this method is
     * called.
     *
     * This method sets atomic flag to false so that all threads exit out
     * infinit loop upon waking up.
     *
     * Without call to this method, the following run time error will be
     * generated:
            "terminate called without an active exception
            Aborted (core dumped)"
     */
    m_Running = false;

    // Wake up the sleeping threads
    m_WorkCondVar.notify_all();

#if 0
    OK if we use line (2) and (3) in SpawnThread() to create threads
    for (auto th : m_Threads)
        th->join();
#endif

    // if we use line (4) and (5) in SpawnThread() to create threads
    for (auto & th : m_Threads)
        th.join();
}

void *thread_start_func(void *p)
{
    return thread_start_func_cpp(p);
}
