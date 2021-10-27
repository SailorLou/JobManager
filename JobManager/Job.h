#pragma once

#include<functional>
#include<atomic>
#include<mutex>
#include<condition_variable>

class JobManager;
class Job
{
public:
    Job() = default;
    Job(const Job&);
    ~Job() { mValid = false; }
    void Initialize(JobManager* parent, std::function<void(Job*)> function = std::function<void(Job*)>(), int refcount = 1);
    void Reset(int refcount = 1); //Call this before reusing a job that doesn't need to be reinitialized
    virtual void Execute();
    void AddReference();
    void RemoveReference();
    void Wait(); //Block until job is finished
private:
    virtual void ExecuteInternal() {}

    std::function<void(Job*)> mFunction;
    JobManager* mParent;
    std::atomic_int mRefCount;

    bool mFinished;
    std::mutex mFinishedLock;
    std::condition_variable mFinishedEvent;
    bool mValid = true;
};

class JobDependency : public Job
{
public:
    void SetDependentJob(Job* job) { mDependendJob = job; }
    virtual void Execute() override
    {
        auto dependendJob = mDependendJob;
        Job::Execute();
        dependendJob->RemoveReference();
    }
private:
    Job* mDependendJob;
};


