#include <bits/stdc++.h>
#include "workers_stl_thread.h"

using namespace std;

int main()
{
    Workers & threadPool = GetThreadPool();

    threadPool.SpawnThread();

    int limit = 100;
    for (int i = 0; i < limit; ++i)
    {
        threadPool.EnqueueWorkItem(new Integer(33+i));

        string str = "Hello World: " + to_string(i);
        threadPool.EnqueueWorkItem(new String(str));

        threadPool.EnqueueWorkItem(new Double(11.11 + i));
    }

    /* Wait for all data to be processed in the Queue */
    threadPool.Wait();
    /* All data is processed */

    // Set up for gracefull exit of threads and this process
    threadPool.TerminateThreadPool();
    
    exit(0);
}
