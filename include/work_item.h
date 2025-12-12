#ifndef WORK_ITEM_H_INCLUDED
#define WORK_ITEM_H_INCLUDED

#include <pthread.h>
#include <bits/stdc++.h>

using namespace std;

class WorkItem
{
    public:
        WorkItem() = default;

        virtual ~WorkItem() {}
        WorkItem(WorkItem &rhs) = delete;
        WorkItem & operator == (const WorkItem &rhs) = delete;

        virtual void process() = 0;
};

class Integer : public WorkItem
{
    private:
        int m_Int;

    public:
        Integer() : WorkItem() { }
        Integer(int d) : WorkItem(), m_Int(d) { }

       void process();
};

class String : public WorkItem
{
    private:
        string m_Str;

    public:
        String() : WorkItem() { }
        String(string & d) : WorkItem(), m_Str(d) { }

       void process();
};

class Double : public WorkItem
{
    private:
        double m_Double;

    public:
        Double() : WorkItem() { }
        Double(double d) : WorkItem(), m_Double(d) { }

       void process();
};

#endif
