#include "NonPreemptiveEDF.hpp"
#include "SchedulerExports.h"

using std::priority_queue;
using std::vector;

void NonPreemptiveEDF::onJobRelease(double time, Job* job) {
	/*if(IsIdle())
		Schedule(job, job->getCost());*/
	// Find an idle processor and execute on that
	for(auto i = 0u; i < GetProcessorCount(); ++i) {
		if(IsIdle(i)) {
			Schedule(i, job, job->getCost());
			break;
		}
	}
}

void NonPreemptiveEDF::onJobFinish(double time, Job* job,unsigned int proc) {
	// I could keep a prioritiy queue but whatever
	union {
		Job** jobList;
		void** jobListGeneric;
	} jList;
	size_t nJobs = 0;
	GetAvailableJobs(nJobs, jList.jobListGeneric);
	if(!nJobs) return;

	Job* highestPriority = nullptr;
	for(size_t i = 0; i < nJobs; ++i) {
		// First one is priority deadline
		// Second thing is checking to see if this job is already on a processor
		printf("Checking job %zd ..", i);
		auto latestProc = jList.jobList[i]->getLatestAssignedProcessor();
		if( ( !highestPriority || jList.jobList[i]->getAbsoluteDeadline() < highestPriority->getAbsoluteDeadline())
			&& ( latestProc < 0 || GetJobOnProcessor( latestProc ) != jList.jobList[i] ) )
			highestPriority = jList.jobList[i];
		printf("Done\n");
	}

	if( highestPriority )
		Schedule(proc, highestPriority, highestPriority->getRemainingCost());
	delete[] jList.jobList;
}

void NonPreemptiveEDF::onJobDeadline(double time, Job* job) {
}

void NonPreemptiveEDF::onJobSliceFinish(double time, Job* job, unsigned int proc) {
}

void NonPreemptiveEDF::onResourceRequest(double time, Job* job, std::string resourceName, unsigned int proc) {
}

void NonPreemptiveEDF::onResourceFinish(double time, Job* job, std::string resourceName, unsigned int proc) {
}

void NonPreemptiveEDF::onTimer(double time, void* timerdata, void* timerpointer) {
}

void NonPreemptiveEDF::onIdle(double time, unsigned int proc) {
}

NonPreemptiveEDF::NonPreemptiveEDF() : Scheduler() {

}

NonPreemptiveEDF::~NonPreemptiveEDF() {

}