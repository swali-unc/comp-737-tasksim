#include "Scheduler.hpp"
#include "SchedulerExports.h"
#include "SimulationState.hpp"

#include <vector>

using std::vector;

DLLEXPORT bool Schedule(Job* job, double duration) {
	return SimulationState::Instance()->getSimulator()->Schedule(job, duration);
}

DLLEXPORT bool StopCurrentJob() {
	return SimulationState::Instance()->getSimulator()->StopExecutingCurrentJob();
}

DLLEXPORT void GetAvailableJobs(size_t& numJobs,Job**& jobPointers) {
	auto currentJobs = SimulationState::Instance()->getSimulator()->getCurrentJobs();
	numJobs = currentJobs.size();
	if(!numJobs)
		return;

	jobPointers = new Job * [numJobs];
	for(auto i = 0u; i < numJobs; ++i)
		jobPointers[i] = currentJobs.at(i);
}

DLLEXPORT bool IsIdle() {
	return SimulationState::Instance()->getSimulator()->IsIdle();
}