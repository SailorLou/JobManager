#include "MultithreadedSolverHelperClass.h"

#include "JobManager.h"


void MultithreadedSolverHelper::Initialize(nv::cloth::Solver* solver, JobManager* jobManager)
{
    mSolver = solver;
    mJobManager = jobManager;
    mEndSimulationJob.Initialize(mJobManager, [this](Job*) {
        mSolver->endSimulation();
        });

    mStartSimulationJob.Initialize(mJobManager, [this](Job*) {
        mSolver->beginSimulation(mDt);
        for (int j = 0; j < mSolver->getSimulationChunkCount(); j++)
            mSimulationChunkJobs[j].RemoveReference();
        });
}

void MultithreadedSolverHelper::StartSimulation(float dt)
{
    mDt = dt;

    if (mSolver->getSimulationChunkCount() != mSimulationChunkJobs.size())
    {
        mSimulationChunkJobs.resize(mSolver->getSimulationChunkCount(), JobDependency());
        for (int j = 0; j < mSolver->getSimulationChunkCount(); j++)
        {
            mSimulationChunkJobs[j].Initialize(mJobManager, [this, j](Job*) {mSolver->simulateChunk(j); });
            mSimulationChunkJobs[j].SetDependentJob(&mEndSimulationJob);
        }
    }
    else
    {
        for (int j = 0; j < mSolver->getSimulationChunkCount(); j++)
            mSimulationChunkJobs[j].Reset();
    }

    mStartSimulationJob.Reset();
    mEndSimulationJob.Reset(mSolver->getSimulationChunkCount());
    mStartSimulationJob.RemoveReference();
}

void MultithreadedSolverHelper::WaitForSimulation()
{
    if (mSolver->getSimulationChunkCount() == 0)
        return;

    mEndSimulationJob.Wait();
}