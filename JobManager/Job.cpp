#include "Job.h"
#include "JobManager.h"
#include "assert.h"

void Job::Initialize(JobManager* parent, std::function<void(Job*)> function, int refcount)
{
    mFunction = function;
    mParent = parent;
    Reset(refcount);
}

Job::Job(const Job& job)
{
    mFunction = job.mFunction;
    mParent = job.mParent;
    mRefCount.store(job.mRefCount);
    mFinished = job.mFinished;
}

void Job::Reset(int refcount)
{
    mRefCount = refcount;
    mFinished = false;
}

void Job::Execute()
{
    if (mFunction)
        mFunction(this);
    else
        ExecuteInternal();

    {
        std::lock_guard<std::mutex> lock(mFinishedLock);
        mFinished = true;
    }
    mFinishedEvent.notify_one();
}

void Job::AddReference()
{
    mRefCount++;
}
void Job::RemoveReference()
{
    int refCount = --mRefCount;
    if (0 == refCount)
    {
        mParent->Submit(this);
    }
    assert(refCount >= 0);
}

void Job::Wait()
{
    std::unique_lock<std::mutex> lock(mFinishedLock);
    mFinishedEvent.wait(lock, [this]() {return mFinished; });
    lock.unlock();
    return;
}
