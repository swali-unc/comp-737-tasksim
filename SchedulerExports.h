#pragma once

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)
#define CEXPORT extern "C" DLLEXPORT
#define CIMPORT extern "C" DLLIMPORT

#if defined(_WIN32) || defined(_WIN64)
#ifdef TASKSIM_EXPORT
#define DLLMODE CEXPORT
#define DLLCLASSMODE DLLEXPORT
#else
#define DLLMODE CIMPORT
#define DLLCLASSMODE DLLIMPORT
#endif
#else
#error If compiling for not-windows, define DLLEXPORT for your shared objects
#endif

DLLMODE bool Schedule(unsigned int proc, void* job, double duration);
DLLMODE bool StopCurrentJob(unsigned int proc);
DLLMODE void* GetJobOnProcessor(unsigned int proc);
DLLMODE bool RecordError(unsigned int proc, const char* str);

DLLMODE void GetAvailableJobs(size_t& numJobs, void**& jobPointers);
DLLMODE bool IsIdle(unsigned int proc);
DLLMODE unsigned int GetProcessorCount();

DLLMODE double GetTime();
DLLMODE void* RegisterTimer(double time, void* callbackPointer);

// If the dll doesn't want to import the class, it can just use these helper exports
DLLMODE double GetJobCost(void* job);
DLLMODE double GetRemainingCost(void* job);
DLLMODE double GetAbsoluteDeadline(void* job);
DLLMODE int GetLatestAssignedProcessor(void* job);
CEXPORT int GetLatestJobAssignedProcessor(void* job);

DLLMODE bool IsJobUsingResource(void* job, const char* resourceName);
DLLMODE void GetJobLabel(void* job, char* buf, size_t bufLen);

DLLMODE int GetTaskIndex(void* job);
DLLMODE int GetJobIndex(void* job);