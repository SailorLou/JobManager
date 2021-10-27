#include "JobManager.h"
void JobManager::WorkerEntryPoint(JobManager* parrent)
{
    while (true)
    {
        Job* job;
        {
            std::unique_lock<std::mutex> lock(parrent->mJobQueueLock);
            while (parrent->mJobQueue.size() == 0 && !parrent->mQuit)
                parrent->mJobQueueEvent.wait(lock);

            if (parrent->mQuit)
                return;

            job = parrent->mJobQueue.front();
            parrent->mJobQueue.pop();
        }
        job->Execute();
    }
}

void JobManager::Submit(Job* job)
{
    mJobQueueLock.lock();
    mJobQueue.push(job);
    mJobQueueLock.unlock();
    mJobQueueEvent.notify_one();
}