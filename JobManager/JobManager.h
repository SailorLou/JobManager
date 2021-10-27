#pragma once

#include"Job.h"
#include<queue>

class JobManager
{
public:
    JobManager()
    {
        mWorkerCount = 8;
        mWorkerThreads = new std::thread[mWorkerCount];
        mQuit = false;

        for (int i = 0; i < mWorkerCount; i++)
            mWorkerThreads[i] = std::thread(JobManager::WorkerEntryPoint, this);
    }
    ~JobManager()
    {
        if (!mQuit)
            Quit();
    }

    void Quit()
    {
        std::unique_lock<std::mutex> lock(mJobQueueLock);
        mQuit = true;
        lock.unlock();
        mJobQueueEvent.notify_all();
        for (int i = 0; i < mWorkerCount; i++)
        {
            mWorkerThreads[i].join();
        }
        delete[] mWorkerThreads;
    }

    template <int count, typename F>
    void ParallelLoop(F const& function)
    {
        /*for(int i = 0; i < count; i++)
            function(i);*/
        Job finalJob;
        finalJob.Initialize(this, std::function<void(Job*)>(), count);
        JobDependency jobs[count];
        for (int j = 0; j < count; j++)
        {
            jobs[j].Initialize(this, [j, &finalJob, function](Job*) {function(j); });
            jobs[j].SetDependentJob(&finalJob);
            jobs[j].RemoveReference();
        }
        finalJob.Wait();
    }

    static void WorkerEntryPoint(JobManager* parrent);
private:
    friend class Job;
    void Submit(Job* job);

    int mWorkerCount;
    std::thread* mWorkerThreads;

    std::mutex mJobQueueLock;
    std::queue<Job*> mJobQueue;
    std::condition_variable mJobQueueEvent;
    bool mQuit;
};

