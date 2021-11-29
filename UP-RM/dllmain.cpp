// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "../SchedulerExports.h"

#pragma comment(lib,"../x64/Release/Comp737TaskSim.lib")

#define REQUIRED_TASKSIM_VERSION 1000ul

CEXPORT unsigned long IdentifyAsScheduler(unsigned long);
CEXPORT void OnJobRelease(double time, void* job);
CEXPORT void OnJobFinish(double time, void* job, unsigned int proc);

void* latestJob = nullptr;
CEXPORT void OnJobRelease(double time, void* job) {
	if (!latestJob) {
		latestJob = job;
		Schedule(0, job, GetRemainingCost(job));
		return;
	}

	if (GetJobPeriod(job) < GetJobPeriod(latestJob)) {
		latestJob = job;
		StopCurrentJob(0);
		Schedule(0, job, GetRemainingCost(job));
	}
}

CEXPORT void OnJobFinish(double time, void* job, unsigned int proc) {
	latestJob = nullptr;

	void** jobList = nullptr;
	size_t nJobs = 0;
	GetAvailableJobs(nJobs, jobList);
	if (!nJobs)
		return;

	for (auto i = 0u; i < nJobs; ++i) {
		if (!latestJob || GetJobPeriod(jobList[i]) < GetJobPeriod(latestJob))
			latestJob = jobList[i];
	}

	delete[] jobList;

	Schedule(0, latestJob, GetRemainingCost(latestJob));
}

CEXPORT unsigned long IdentifyAsScheduler(unsigned long) {
	return REQUIRED_TASKSIM_VERSION;
}

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

