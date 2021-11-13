// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "../SchedulerExports.h"

#pragma comment(lib,"../x64/Release/Comp737TaskSim.lib")

#define REQUIRED_TASKSIM_VERSION 1u

CEXPORT void OnJobRelease(double time, void* job) {
	void* lowestPriorityJob = nullptr;

	for (auto i = 0u; i < GetProcessorCount(); ++i) {
		if (IsIdle(i)) {
			Schedule(i, job, GetRemainingCost(job));
			return;
		}

		auto procJob = GetJobOnProcessor(i);
		if (!lowestPriorityJob || GetAbsoluteDeadline(procJob) > GetAbsoluteDeadline(lowestPriorityJob))
			lowestPriorityJob = procJob;
	}

	if (GetAbsoluteDeadline(job) < GetAbsoluteDeadline(lowestPriorityJob)) {
		auto proc = GetLatestAssignedProcessor(lowestPriorityJob);
		StopCurrentJob(proc);
		Schedule(proc, job, GetRemainingCost(job));
	}
}

CEXPORT void OnJobFinish(double time, void* job, unsigned int proc) {
	size_t numJobs;
	void** currentJobs;
	GetAvailableJobs(numJobs, currentJobs);
	if (!numJobs) return;

	void* highestPriorityJob = nullptr;
	for (auto i = 0u; i < numJobs; ++i) {
		auto latestProc = GetLatestAssignedProcessor(currentJobs[i]);
		/*if (!highestPriorityJob || (
			( latestProc < 0 || GetJobOnProcessor(latestProc) != currentJobs[i] )
			&& GetAbsoluteDeadline(currentJobs[i]) < GetAbsoluteDeadline(highestPriorityJob)))*/
		if ((!highestPriorityJob || GetAbsoluteDeadline(currentJobs[i]) < GetAbsoluteDeadline(highestPriorityJob))
			&& (latestProc < 0 || GetJobOnProcessor(latestProc) != currentJobs[i]))
			highestPriorityJob = currentJobs[i];
	}

	if( highestPriorityJob )
		Schedule(proc, highestPriorityJob, GetRemainingCost(highestPriorityJob));
	delete[] currentJobs;
}

CEXPORT unsigned long IdentifyAsScheduler(unsigned long) {
	return REQUIRED_TASKSIM_VERSION;
}

BOOL APIENTRY DllMain(	HMODULE hModule,
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

