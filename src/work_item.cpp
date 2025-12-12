#include "work_item.h"
#include "workers_stl_thread.h"

using namespace std;

void Integer::process()
{
    char s[180];
    snprintf(s, sizeof(s), "Integer::process: %d, ThreadID=%s\n", m_Int,
            Workers::getThreadId().c_str());
    cout << s;
} 

void String::process()
{
   char s[180];
   snprintf(s, sizeof(s), "String::process: %s, ThreadID=%s\n", 
            m_Str.c_str(), Workers::getThreadId().c_str());
   cout << s;
   WorkItem *w = new Integer(-7777);
   GetThreadPool().EnqueueWorkItem(w);
} 

void Double::process()
{
    char s[180];
    snprintf(s, sizeof(s), "Double: Consumer Thread=%s is now a producer", 
             Workers::getThreadId().c_str());

    string str = s;
    WorkItem * w = new String(str);
    GetThreadPool().EnqueueWorkItem(w);

    snprintf(s, sizeof(s), "Double::process: %lf, ThreadID=%s\n", 
            m_Double, Workers::getThreadId().c_str());
    cout << s;
} 
