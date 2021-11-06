#include "NonPreemptiveEDF.hpp"
#include "SchedulerExports.h"

using std::priority_queue;
using std::vector;

void NonPreemptiveEDF::onJobRelease(double time, Job* job) {
	if(IsIdle())
		Schedule(job, job->getCost());
}

void NonPreemptiveEDF::onJobFinish(double time, Job* job) {
	// I could keep a prioritiy queue but whatever
	Job** jobList = nullptr;
	size_t nJobs = 0;
	GetAvailableJobs(nJobs, jobList);
	if(!nJobs) return;

	Job* highestPriority = jobList[0];
	for(size_t i = 1; i < nJobs; ++i) {
		if(jobList[i]->getAbsoluteDeadline() < highestPriority->getAbsoluteDeadline())
			highestPriority = jobList[i];
	}

	Schedule(highestPriority, highestPriority->getRemainingCost());
	delete[] jobList;
}

void NonPreemptiveEDF::onJobDeadline(double time, Job* job) {
}

void NonPreemptiveEDF::onResourceRequest(double time, Job* job, std::string resourceName) {
}

void NonPreemptiveEDF::onResourceFinish(double time, Job* job, std::string resourceName) {
}

void NonPreemptiveEDF::onTimer(double time, void* timerdata) {
}

void NonPreemptiveEDF::onIdle(double time) {
}

NonPreemptiveEDF::NonPreemptiveEDF() : Scheduler() {

}

NonPreemptiveEDF::~NonPreemptiveEDF() {

}