// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "../SchedulerExports.h"

#pragma comment(lib,"../x64/Release/Comp737TaskSim.lib")

#define REQUIRED_TASKSIM_VERSION 1u

void* tieBreaker(void* job1, void* job2);

CEXPORT void OnJobRelease(double time, void* job);
CEXPORT void OnJobFinish(double time, void* job, unsigned int proc);

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

	auto dl1 = GetAbsoluteDeadline(job);
	auto dl2 = GetAbsoluteDeadline(lowestPriorityJob);
	if (dl1 == dl2) {
		if(GetRemainingCost(lowestPriorityJob) > 0)
			return; // prioritize the currently executing job
		auto realjob = tieBreaker(lowestPriorityJob, job);
		if (realjob != lowestPriorityJob)
			return;
		auto proc = GetLatestAssignedProcessor(lowestPriorityJob);
		StopCurrentJob(proc);
		Schedule(proc, realjob, GetRemainingCost(realjob));
	}

	//if (GetAbsoluteDeadline(job) < GetAbsoluteDeadline(lowestPriorityJob)) {
	if(dl1 < dl2) {
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
		if (latestProc < 0 || GetJobOnProcessor(latestProc) != currentJobs[i]) {
			if (!highestPriorityJob) {
				highestPriorityJob = currentJobs[i];
				continue;
			}

			auto dl1 = GetAbsoluteDeadline(currentJobs[i]);
			auto dl2 = GetAbsoluteDeadline(highestPriorityJob);
			if (dl1 == dl2) {
				// break tie with task id if it exists
				highestPriorityJob = tieBreaker(currentJobs[i], highestPriorityJob);
			}
			else if (dl1 < dl2)
				highestPriorityJob = currentJobs[i];
		}
	}

	if( highestPriorityJob )
		Schedule(proc, highestPriorityJob, GetRemainingCost(highestPriorityJob));
	delete[] currentJobs;
}

CEXPORT unsigned long IdentifyAsScheduler(unsigned long) {
	return REQUIRED_TASKSIM_VERSION;
}

void* tieBreaker(void* job1, void* job2) {
	// break tie with task id if it exists
	int tid1 = GetTaskIndex(job1);
	int tid2 = GetTaskIndex(job2);
	if (tid1 == -1)
		tid1 = GetJobIndex(job1);
	if (tid2 == -1)
		tid2 = GetJobIndex(job2);
	if (tid1 == tid2)
		return (size_t)job1 < (size_t)job2 ? job1 : job2;
	return (tid1 < tid2) ? job1 : job2;
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

