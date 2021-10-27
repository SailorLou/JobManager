#pragma once

#include<vector>
#include"Job.h"

class JobManager;

class MultithreadedSolverHelper
{
public:
    void Initialize(nv::cloth::Solver* solver, JobManager* jobManager);
    void StartSimulation(float dt);
    void WaitForSimulation();
private:
    Job mStartSimulationJob;
    Job mEndSimulationJob;
    std::vector<JobDependency> mSimulationChunkJobs;

    float mDt;

    nv::cloth::Solver* mSolver;
    JobManager* mJobManager;
};

