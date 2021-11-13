#include "Scheduler.hpp"
#include "SchedulerExports.h"
#include "SimulationState.hpp"

#include <vector>
#include <string>

using std::vector;
using std::string;

void RecordError(const char* text);

CEXPORT bool Schedule(unsigned int proc, void* job, double duration) {
	if(duration < 0) {
		RecordError("Schedule: Negative duration given");
		return false;
	}

	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError("Schedule given bad processor index");
		return false;
	}

	return SimulationState::Instance()->getSimulator()->Schedule((Job*)job, duration, proc);
}

CEXPORT bool RecordError(unsigned int proc, const char* str) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError("Record error given bad processor index");
		return false;
	}
	
	SimulationState::Instance()->getSimulator()->logScheduleError(str, proc);
	return true;
}

void RecordError(const char* text) {
	RecordError(0, text);
}

CEXPORT bool StopCurrentJob(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError("StopCurrentJob given bad processor index");
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
		RecordError("IsIdle given bad processor index");
		return false;
	}
	return SimulationState::Instance()->getSimulator()->IsIdle(proc);
}

CEXPORT unsigned int GetProcessorCount() {
	return SimulationState::Instance()->getProblem()->getProcessorCount();
}

CEXPORT void* GetJobOnProcessor(unsigned int proc) {
	if(proc >= SimulationState::Instance()->getProblem()->getProcessorCount()) {
		RecordError("GetJobOnProcessor given bad processor index");
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
	// The reason we do this is because the simulator only progresses the job pointer
	//  accumulated runtime when it needs to
	return SimulationState::Instance()->getSimulator()->getRemainingCostOfJob((Job*)job);
}

CEXPORT double GetAbsoluteDeadline(void* job) {
	return ((Job*)job)->getAbsoluteDeadline();
}

CEXPORT void* RegisterTimer(double time, void* callbackPointer) {
	return SimulationState::Instance()->getSimulator()->registerTimer(time, callbackPointer);
}

CEXPORT double GetTime() {
	return SimulationState::Instance()->getSimulator()->getTime();
}

CEXPORT bool IsJobUsingResource(void* job, const char* name) {
	return SimulationState::Instance()->getSimulator()->isJobUsingResource((Job*)job, name);
}

CEXPORT void GetJobLabel(void* job, char* buf, size_t bufLen) {
	auto str = ((Job*)job)->createLabel();
	strcpy_s(buf, bufLen, str.c_str());
}