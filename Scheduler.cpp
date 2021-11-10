#include "Scheduler.hpp"
#include "SchedulerExports.h"
#include "SimulationState.hpp"

#include <vector>

using std::vector;

DLLEXPORT bool Schedule(unsigned int proc, void* job, double duration) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "Schedule given bad processor index");
		return false;
	}

	return SimulationState::Instance()->getSimulator()->Schedule((Job*)job, duration, proc);
}

DLLEXPORT bool RecordError(unsigned int proc, const char* str) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "Record error given bad processor index");
		return false;
	}
	
	SimulationState::Instance()->getSimulator()->logScheduleError(str, proc);
	return true;
}

DLLEXPORT bool StopCurrentJob(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "StopCurrentJob given bad processor index");
		return false;
	}
	return SimulationState::Instance()->getSimulator()->StopExecutingCurrentJob(proc);
}

DLLEXPORT void GetAvailableJobs(size_t& numJobs, void**& jobPointers) {
	auto currentJobs = SimulationState::Instance()->getSimulator()->getCurrentJobs();
	numJobs = currentJobs.size();
	if(!numJobs)
		return;

	auto jobList = new Job * [numJobs];
	for(size_t i = 0; i < numJobs; ++i)
		jobList[i] = currentJobs.at(i);
	jobPointers = (void**)jobList;
}

DLLEXPORT bool IsIdle(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "IsIdle given bad processor index");
		return false;
	}
	return SimulationState::Instance()->getSimulator()->IsIdle(proc);
}

DLLEXPORT unsigned int GetProcessorCount() {
	return SimulationState::Instance()->getProblem()->getProcessorCount();
}

DLLEXPORT void* GetJobOnProcessor(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError(0, "GetJobOnProcessor given bad processor index");
		return nullptr;
	}
	return (void*)SimulationState::Instance()->getSimulator()->GetJobOnProcessor(proc);
}