#ifndef WORKERS_H_INCLUDE
#define WORKERS_H_INCLUDE

#include <bits/stdc++.h>
#include "work_item.h"

using namespace std;

void* thread_start_func_cpp (void* p);

extern "C"
{
void* thread_start_func (void* p);
}

class Workers
{
    private:
        int m_NumWorkers;
        std::atomic<bool> m_Running;
        vector<std::thread> m_Threads;
        //vector<std::thread *> m_Threads;
        //mutable std::mutex m_Mutex;
        mutable std::recursive_mutex m_Mutex;
        //std::condition_variable m_WorkCondVar; condition_variable works
        //with std::mutex only. It does not work other mutex like shared_mutex, recursive_mutex
        //timed_mutex etc. condition_variable_any supports all kinds of
        //mutex.
        std::condition_variable_any m_WorkCondVar;
        //std::condition_variable m_WaitCondVar;
        std::condition_variable_any m_WaitCondVar;
        queue<WorkItem *> m_WorkItems;

        void ThreadStart();
    public:
        friend void *thread_start_func_cpp(void *p);

        Workers(int NumThreads);
#if 0
        static Workers & Instance(int NumThread = 5)
        {
            static Workers workers(NumThread);

            return workers;
        }
#endif
        ~Workers();

        Workers(Workers &rhs) = delete;
        Workers & operator = (const Workers & rhs) = delete;

        int GetNumJobs() const
        {
            //std::lock_guard<std::mutex> lkg(m_Mutex);
            std::lock_guard<std::recursive_mutex> lkg(m_Mutex);
            return static_cast<int>(m_WorkItems.size());
        }

        void SpawnThread();

        void EnqueueWorkItem(WorkItem *item);

        void Wait();

        void TerminateThreadPool();

        static string getThreadId()
        {
            std::thread::id threadId = std::this_thread::get_id();

            std::ostringstream oss;
            oss << threadId;
            return oss.str();
        }
};

//extern "C"
//{
Workers & GetThreadPool();
//}
#endif
