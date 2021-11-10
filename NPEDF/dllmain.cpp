// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "SchedulerExports.h"

#pragma comment(lib,"../x64/Release/Comp737TaskSim.lib")

#define REQUIRED_TASKSIM_VERSION 1ul

CEXPORT void OnJobRelease(double time, void* job) {
    for(auto i = 0u; i < GetProcessorCount(); ++i) {
        if(IsIdle(i)) {
            Schedule(i, job, GetJobCost(job));
            break;
        }
    }

    static bool once = false;
    if(!once && time > 5) {
        once = true;
        RecordError(0, "test error comment");
    }
}

CEXPORT void OnJobFinish(double time, void* job, unsigned int proc) {
	// I could keep a prioritiy queue but whatever
    void** jobList = nullptr;
	size_t nJobs = 0;
	GetAvailableJobs(nJobs, jobList);
	if(!nJobs) return;

	void* highestPriority = nullptr;
	for(size_t i = 0; i < nJobs; ++i) {
		// First one is priority deadline
		// Second thing is checking to see if this job is already on a processor
		//auto latestProc = jList.jobList[i]->getLatestAssignedProcessor();
        auto latestProc = GetLatestAssignedProcessor(jobList[i]);
		if((!highestPriority || GetAbsoluteDeadline(jobList[i]) < GetAbsoluteDeadline(highestPriority))
			&& (latestProc < 0 || GetJobOnProcessor(latestProc) != jobList[i] ))
			highestPriority = jobList[i];
	}

	if(highestPriority)
		Schedule(proc, highestPriority, GetRemainingCost(highestPriority));
	delete[] jobList;
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

