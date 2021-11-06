#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define DLLEXPORT __declspec(dllexport)
#else
#error If compiling for not-windows, define DLLEXPORT for your shared objects
#endif

#include "Job.hpp"

DLLEXPORT bool Schedule(Job* job, double duration);
DLLEXPORT bool StopCurrentJob();
DLLEXPORT void GetAvailableJobs(size_t& numJobs, Job**& jobPointers);
DLLEXPORT bool IsIdle();