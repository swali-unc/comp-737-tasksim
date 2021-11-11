// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "../SchedulerExports.h"

#pragma comment(lib,"../x64/Release/Comp737TaskSim.lib")

#define REQUIRED_TASKSIM_VERSION 1ul

#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>
#include <utility>
#include <map>

using std::vector;
using std::pair;
using std::string;
using std::make_pair;
using std::map;

struct jobData {
	double absoluteDeadline;
	double currentPriorityDeadline;
	bool blocked;
	vector<pair<string, void*>> inheritedPriorities;
};
map<void*, jobData> currentJobs;
void* currentHighestPriority = nullptr;

void doEDF();
void addPriorityInheritence(void* job, void* waitingJob, string resourceName);
void removePriorityInheritence(void* job, string resourceName);

CEXPORT void OnJobRelease(double time, void* job) {
	auto currentDeadline = GetAbsoluteDeadline(job);
	currentJobs[job] = jobData{ currentDeadline, currentDeadline, false };

	// No highest priority?
	if(!currentHighestPriority) {
		// Schedule this guy then!
		currentHighestPriority = job;
		Schedule(0, job, GetRemainingCost(job));
		return;
	}

	// Check to see if new release has higher priority
	if(currentJobs[currentHighestPriority].currentPriorityDeadline < currentDeadline) {
		// We have a new highest priority, so execute this
		currentHighestPriority = job;
		StopCurrentJob(0);
		Schedule(0, job, GetRemainingCost(job));
		return;
	}

	// Check EDF
	doEDF();
}

CEXPORT void OnJobFinish(double time, void* job, unsigned int proc) {
	currentJobs.erase(currentJobs.find(job));
	currentHighestPriority = nullptr;

	// find next highest priority job
	doEDF();
}

CEXPORT void OnJobResourceRequest(double time, void* job, const char* resourceName, unsigned int proc) {
	// Is anyone else using this resource?
	void** jobList = nullptr;
	size_t nJobs = 0;
	GetAvailableJobs(nJobs, jobList);
	
	for(size_t i = 0; i < nJobs; ++i) {
		if(jobList[i] == job) continue;

		if(IsJobUsingResource(jobList[i], resourceName)) {
			// This job is using it! They must be lower priority.
			addPriorityInheritence(jobList[i], job, resourceName);
			currentJobs[job].blocked = true;
		}
	}

	// Determine EDF
	doEDF();

	if(currentHighestPriority != job) {
		// TODO: comment about priority inversion, another job stole our priority
	}
}

CEXPORT void OnJobResourceFinish(double time, void* job, const char* resourceName, unsigned int proc) {
	removePriorityInheritence(job, resourceName);

	// Determine EDF
	doEDF();
}

void doEDF() {
	void* newHighestPriority = currentHighestPriority;
	for(auto& i : currentJobs) {
		if(!newHighestPriority ||
			(i.second.currentPriorityDeadline <= currentJobs[newHighestPriority].currentPriorityDeadline
			&& !i.second.blocked)
			) {
			newHighestPriority = i.first;
		}
	}

	// No new highest priority? do nothing, let highest priority keep executing
	if(newHighestPriority == currentHighestPriority)
		return;
	currentHighestPriority = newHighestPriority;
	if(!newHighestPriority) // No jobs left?
		return;
	// Otherwise, stop the current job if any, then do this new highest priority job
	if(!IsIdle(0))
		StopCurrentJob(0);
	Schedule(0, currentHighestPriority, GetRemainingCost(currentHighestPriority));
}

void addPriorityInheritence(void* job, void* waitingJob, string resourceName) {
	// Add the waiting job to our list of inherited job priorities
	currentJobs[job].inheritedPriorities.push_back(pair<string, void*>(resourceName, waitingJob));

	// Is this job even higher than our current priority?
	if(currentJobs[job].currentPriorityDeadline > currentJobs[waitingJob].currentPriorityDeadline)
		currentJobs[job].currentPriorityDeadline = currentJobs[waitingJob].currentPriorityDeadline;
}

void removePriorityInheritence(void* job, string resourceName) {
	double soonest = currentJobs[job].absoluteDeadline;

	// Go through all of our inherited priorities
	for( auto it = currentJobs[job].inheritedPriorities.begin();
		it != currentJobs[job].inheritedPriorities.end(); ) {

		// Is this priority inherited from the released resource?
		if(it->first == resourceName) {
			// Unblock all jobs waiting on this resource
			currentJobs[it->second].blocked = false;

			// remove this
			currentJobs[job].inheritedPriorities.erase(it);
		}
		else {
			// Is this inherited job's deadline even sooner? then we inherit this deadline
			if(currentJobs[it->second].currentPriorityDeadline < soonest)
				soonest = currentJobs[it->second].currentPriorityDeadline;
			++it;
		}
	}

	// Set our current priority
	currentJobs[job].currentPriorityDeadline = soonest;
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

