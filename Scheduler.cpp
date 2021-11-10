#include "Scheduler.hpp"
#include "SchedulerExports.h"
#include "SimulationState.hpp"

#include <vector>

using std::vector;

CEXPORT bool Schedule(unsigned int proc, void* job, double duration) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "Schedule given bad processor index");
		return false;
	}

	return SimulationState::Instance()->getSimulator()->Schedule((Job*)job, duration, proc);
}

CEXPORT bool RecordError(unsigned int proc, const char* str) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "Record error given bad processor index");
		return false;
	}
	
	SimulationState::Instance()->getSimulator()->logScheduleError(str, proc);
	return true;
}

CEXPORT bool StopCurrentJob(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "StopCurrentJob given bad processor index");
		return false;
	}
	return SimulationState::Instance()->getSimulator()->StopExecutingCurrentJob(proc);
}

CEXPORT void GetAvailableJobs(size_t& numJobs, void**& jobPointers) {
	auto currentJobs = SimulationState::Instance()->getSimulator()->getCurrentJobs();
	numJobs = currentJobs.size();
	if(!numJobs)
		return;

	auto jobList = new Job * [numJobs];
	for(size_t i = 0; i < numJobs; ++i)
		jobList[i] = currentJobs.at(i);
	jobPointers = (void**)jobList;
}

CEXPORT bool IsIdle(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "IsIdle given bad processor index");
		return false;
	}
	return SimulationState::Instance()->getSimulator()->IsIdle(proc);
}

CEXPORT unsigned int GetProcessorCount() {
	return SimulationState::Instance()->getProblem()->getProcessorCount();
}

CEXPORT void* GetJobOnProcessor(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "GetJobOnProcessor given bad processor index");
		return nullptr;
	}
	return (void*)SimulationState::Instance()->getSimulator()->GetJobOnProcessor(proc);
}

CEXPORT double GetJobCost(void* job) {
	return ((Job*)job)->getCost();
}

CEXPORT int GetLatestAssignedProcessor(void* job) {
	return ((Job*)job)->getLatestAssignedProcessor();
}

CEXPORT double GetRemainingCost(void* job) {
	return ((Job*)job)->getRemainingCost();
}

CEXPORT double GetAbsoluteDeadline(void* job) {
	return ((Job*)job)->getAbsoluteDeadline();
}