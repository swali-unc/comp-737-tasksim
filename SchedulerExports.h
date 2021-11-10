#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define DLLEXPORT __declspec(dllexport)
#else
#error If compiling for not-windows, define DLLEXPORT for your shared objects
#endif

#include "Job.hpp"

DLLEXPORT bool Schedule(unsigned int proc, void* job, double duration);
DLLEXPORT bool StopCurrentJob(unsigned int proc);
DLLEXPORT void* GetJobOnProcessor(unsigned int proc);
DLLEXPORT bool RecordError(unsigned int proc, const char* str);

DLLEXPORT void GetAvailableJobs(size_t& numJobs, void**& jobPointers);
DLLEXPORT bool IsIdle(unsigned int proc);
DLLEXPORT unsigned int GetProcessorCount();