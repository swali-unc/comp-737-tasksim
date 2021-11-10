#include "TaskSimulator.hpp"

#include <algorithm>
#include "JobStatusEvent.hpp"
#include "CommentEvent.hpp"
#include "Utility.hpp"
#include "SimulationState.hpp"

using std::find;
using std::string;
using std::vector;
using std::pair;

bool TaskSimulator::Simulate() {
	auto sched = SimulationState::Instance()->getScheduler();
	if (!sched) {
		logScheduleError("No scheduler loaded",0);
		return false;
	}

	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();
	auto nextEvent = upcomingEventQueue.top();
	auto nextEventTime = nextEvent ? nextEvent->getStart() : SimulationState::Instance()->getProblem()->getTimelineInterval();
	//printf("Initial next event time %f\n", nextEventTime);
	//bool nextEventIsCompletion = false, nextEventIsResourceAccess = false, nextEventIsResourceRelease = false;
	enum _nextEventType {
		UNKNOWN, COMPLETION, RESOURCEACCESS, RESOURCERELEASE, TIMEFINISH
	};
	_nextEventType nextEventType = UNKNOWN;
	string resource;
	//printf("Next event time: %f\n", nextEventTime);

	auto nextEventProc = 0u;
	for(auto i = 0u; i < procCount; ++i) {
		if(currentJobOnProc[i]) {
			auto remainingCost = currentJobOnProc[i]->getRemainingCost();

			// Will we hit a resource access/release?
			string resourceName, resourceReleaseName;
			auto timeTillResource = currentJobOnProc[i]->getNextResourceAccess(resourceName);
			auto timeTillResourceRelease = currentJobOnProc[i]->getNextResourceRelease(resourceReleaseName);

			if(timeTillResourceRelease > 0 && time + timeTillResourceRelease < nextEventTime) {
				nextEventType = RESOURCERELEASE;
				resource = resourceName;
				nextEventTime = time + timeTillResourceRelease;
				nextEventProc = i;
			}

			if(timeTillResource > 0 && time + timeTillResource < nextEventTime) {
				nextEventTime = time + timeTillResource;
				nextEventType = RESOURCEACCESS;
				resource = resourceName;
				nextEventProc = i;
			}

			if(currentJobStart[i] + remainingCost < nextEventTime) {
				// Our current job will finish executing before the next event
				// so the real next event is a job completion
				nextEventType = COMPLETION;
				nextEventTime = currentJobStart[i] + remainingCost;
				nextEventProc = i;
			}

			if(currentJobStart[i] + currentDuration[i] < nextEventTime) {
				nextEventType = TIMEFINISH;
				nextEventTime = currentJobStart[i] + currentDuration[i];
				nextEventProc = i;
			}

			//printf("Set next time: %f (%d) - %u has %f+%f (%s)\n", nextEventTime, nextEventType, i,
			//	currentJobStart[i], remainingCost,
			//	currentJobOnProc[i]->createLabel().c_str());
		}
	}

	//printf("%f: next event type %d @ %f on proc %u\n", time, nextEventType, nextEventTime, nextEventProc);

	// Do we have a timer that will hit before any of the current events?
	pair<double, void*>* earliestTimer = nullptr;
	for(auto& i : timers) {
		// This needs to be a timer in the future, and less than our next event
		if(i->first > time && time - i->first < nextEventTime) {
			nextEventTime = i->first;
			earliestTimer = i;
		}
	}

	auto elapsedTime = nextEventTime - time;
	auto startTime = time;
	time = nextEventTime;

	if(earliestTimer) {
		// We had a timer go off
		sched->onTimer(time, earliestTimer->second, (void*)earliestTimer);
		return true;
	}

	// Execute on all jobs on all processors by that time
	/*for(auto i = 0u; i < procCount; ++i) {
		if(currentJobOnProc[i]) {
		}
	}*/

	// We have a job event and not a regular event
	if(nextEventType != UNKNOWN) {
		auto job = currentJobOnProc[nextEventProc];

		// Job finish event
		if(nextEventType == COMPLETION) {
			schedules[nextEventProc].push_back(job->executeJob(time - currentJobStart[nextEventProc], currentJobStart[nextEventProc]));

			schedules[nextEventProc].push_back(new JobFinishEvent(time, *job));
			currentJobs.erase(std::find(currentJobs.begin(), currentJobs.end(), job));
			currentJobOnProc[nextEventProc] = nullptr;
			//printf("[%f] Cleared job %s\n", time, job->createLabel().c_str());
			sched->onJobFinish(time, job, nextEventProc);
			//delete job;
			return true;
		}
		else if(nextEventType == RESOURCEACCESS) {
			sched->onResourceRequest(time, job, resource, nextEventProc);
			return true;
		}
		else if(nextEventType == RESOURCERELEASE) {
			sched->onResourceFinish(time, job, resource, nextEventProc);
			return true;
		}
		else if(nextEventType == TIMEFINISH) {
			sched->onJobSliceFinish(time, job, nextEventProc);
			StopExecutingCurrentJob(nextEventProc);
			return true;
		}
	}

	// There was no next event, we're at the end?
	if (!nextEvent) {
		// Find idle-ness and notify our scheduler
		for(auto i = 0u; i < procCount; ++i) {
			if(IsIdle(i))
				sched->onIdle(time,i);
		}

		if(time >= SimulationState::Instance()->getProblem()->getScheduleLength()) {
			// We're at the end, process all jobs and finish up
		}
		return true;
	}

	// Remove this event
	upcomingEventQueue.pop();

	// Is it a job release?
	// If these events have a processor previously associated with them, great, otherwise
	//  we by default show the event on processor 0
	auto eventType = nextEvent->getType();
	if (eventType == ScheduleEventType::ReleaseEvent) {
		JobReleaseEvent* releaseEvent = static_cast<JobReleaseEvent*>(nextEvent);
		int latestProc = releaseEvent->getJob()->getLatestAssignedProcessor();
		auto proc = latestProc < 0 ? 0u : (unsigned int)latestProc;

		schedules[proc].push_back(releaseEvent);
		currentJobs.push_back(releaseEvent->getJob());
		sched->onJobRelease(time, releaseEvent->getJob());
		return true;
	}

	// Did we get a deadline?
	if (eventType == ScheduleEventType::DeadlineEvent) {
		JobDeadlineEvent* deadlineEvent = static_cast<JobDeadlineEvent*>(nextEvent);
		int latestProc = deadlineEvent->getJob()->getLatestAssignedProcessor();
		auto proc = latestProc < 0 ? 0u : (unsigned int)latestProc;

		schedules[proc].push_back(deadlineEvent);

		// Did we miss a deadline?
		if (deadlineEvent->getJob()->getRemainingCost() > 0) {
			logScheduleError(stringprintf("Deadline missed for %s (%f)",
				deadlineEvent->getJob()->createLabel().c_str(), time), proc);
		}

		sched->onJobDeadline(time, deadlineEvent->getJob());
		return true;
	}

	logScheduleError(stringprintf("Error, unknown event type: %d", eventType), 0);
	return false;
}

void TaskSimulator::LoadProblem(ProblemSet* problem) {
	Reset();

	if(!problem)
		problem = SimulationState::Instance()->getProblem();

	auto taskSet = problem->getTaskSet();
	auto taskCount = problem->getTaskCount();
	auto jobSet = problem->getAperiodics();
	auto jobCount = problem->getAperiodicCount();
	auto problemLength = problem->getScheduleLength();

	// Load all job releases and deadlines into upcoming schedule
	for (auto i = 0u; i < taskCount; ++i) {
		double phase = taskSet[i]->getPhase();
		double period = taskSet[i]->getPeriod();

		int jobIndex = 0;
		for (double t = phase; t < problemLength; t += period) {
			Job* newJob = new Job(*taskSet[i], t, jobIndex++);
			upcomingJobReleases.push(newJob);
			upcomingEventQueue.push(new JobReleaseEvent(*newJob));
			upcomingEventQueue.push(new JobDeadlineEvent(*newJob));
		}
	}

	// Now do aperiodics
	for (auto i = 0u; i < jobCount; ++i) {
		Job* newJob = new Job(*jobSet[i]);
		upcomingJobReleases.push(newJob);
		upcomingEventQueue.push(new JobReleaseEvent(*newJob));
		upcomingEventQueue.push(new JobDeadlineEvent(*newJob));
	}

	// TODO: Now do sporadic
}

bool TaskSimulator::Schedule(Job* job, double duration, unsigned int proc) {
	if(!job) return false;

	if (currentJobOnProc[proc]) {
		if (currentJobOnProc[proc] == job) {
			// If it's the same job, then no need to do anything
			//double currentDuration = currentJob->getDuration();
			//currentJob->setDuration((currentDuration - (time - currentJobStart)) + duration);
			//if(currentJobStart[proc] + currentDuration[proc] > time + duration)
			currentDuration[proc] = (time - currentJobStart[proc]) + duration;
			return true;
		}

		// End the current job execution at this time
		StopExecutingCurrentJob(proc);
	}

	// Can't schedule this job if it isn't in our current job list
	if (find(currentJobs.begin(), currentJobs.end(), job) == currentJobs.end()) {
		logScheduleError(stringprintf(
			"Simulation Error: Algorithm tried to schedule job %s that has either not released yet or is unknown",
			job->createLabel().c_str()
		), proc );
		return false;
	}

	// Is this job on another processor already?
	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();
	for(auto i = 0u; i < procCount; ++i) {
		if(i == proc) continue;
		if(currentJobOnProc[i] == job) {
			logScheduleError(stringprintf(
				"Simulation Error: Algorithm tried to schedule job %s on processor %u that is executing on a different processor %u",
				job->createLabel().c_str(), proc, i
			), proc);
			return false;
		}
	}

	// schedule our new job
	//currentJob = new JobExecution(*job, time, duration);
	currentJobOnProc[proc] = job;
	currentJobStart[proc] = time;
	currentDuration[proc] = duration;
	job->setAssignedProcessor(proc);
	// Also need to find all other jobs with the same task
	/*if(job->getRelatedTask()) {
		for(auto& i : getCurrentJobs()) {
			if(job->getRelatedTask() == i->getRelatedTask())
				job->setAssignedProcessor(proc);
		}
	}*/
	//printf("%s assigned to proc %u for %f @ %f\n", job->createLabel().c_str(), proc, duration, time);
	//schedule.push_back(currentJob);
	return true;
}

bool TaskSimulator::StopExecutingCurrentJob(unsigned int proc) {
	if (!currentJobOnProc[proc]) {
		logScheduleError("Attempted to stop executing current job on processor, but no job currently is executing.",proc);
		return false;
	}

	//currentJob->setDuration(time - currentJobStart);
	if(currentJobStart[proc] < time) {
		schedules[proc].push_back(currentJobOnProc[proc]->executeJob(time - currentJobStart[proc], currentJobStart[proc]));
	}

	currentJobOnProc[proc] = nullptr;
	return true;
}

double TaskSimulator::getRemainingCostOfJob(Job* job) {
	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();

	// Is this job on a processor?
	for(auto i = 0u; i < procCount; ++i) {
		if(currentJobOnProc[i] == job) {
			// This is an active job, lets find out how much time is remaining
			auto elapsedTime = (time - currentJobStart[i]);
			return job->getRemainingCost() - elapsedTime;
		}
	}

	return job->getRemainingCost();
}

void* TaskSimulator::registerTimer(double delta, void* callbackData) {
	auto newTimer = new pair<double, void*>(time + delta, callbackData);
	timers.push_back(newTimer);
	return (void*)newTimer;
}

void TaskSimulator::logScheduleError(string errorText, unsigned int proc) {
	schedules[proc].push_back(new CommentEvent(time, errorText));
	SimulationState::Instance()->logError(time, errorText);
	fprintf(stderr, "[Proc %u] Scheduler error reported: %s\n", proc, errorText.c_str());
}

void TaskSimulator::Reset() {
	time = 0;
	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();
	//printf("proc count: %u\n", procCount);
	Destroy();
	//printf("destroy done\n");

	currentJobOnProc = new Job* [procCount];
	for(auto i = 0u; i < procCount; ++i)
		currentJobOnProc[i] = nullptr;

	schedules = new vector<ScheduleEvent*>[procCount];
	currentDuration = new double[procCount];
	currentJobStart = new double[procCount];
}

void TaskSimulator::Destroy() {
	if(currentJobOnProc) {
		delete[] currentJobOnProc;
		currentJobOnProc = nullptr;
	}
	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();

	while(upcomingEventQueue.size()) {
		delete upcomingEventQueue.top();
		upcomingEventQueue.pop();
	}

	while(currentJobs.size()) {
		delete currentJobs[currentJobs.size() - 1];
		currentJobs.pop_back();
	}

	while(upcomingJobReleases.size()) {
		delete upcomingJobReleases.top();
		upcomingJobReleases.pop();
	}

	if(schedules) {
		for(auto i = 0u; i < procCount; ++i) {
			while(schedules[i].size()) {
				delete schedules[i][schedules[i].size() - 1];
				schedules[i].pop_back();
			}
		}

		delete[] schedules;
		schedules = nullptr;
	}

	if(currentJobStart)
		delete[] currentJobStart;
	if(currentDuration)
		delete[] currentDuration;

	for(auto& i : timers)
		delete i;
	timers.clear();
}

TaskSimulator::TaskSimulator() noexcept {
	currentJobOnProc = nullptr;
	schedules = nullptr;
	currentJobStart = nullptr;
	currentDuration = nullptr;
	Reset();
}

TaskSimulator::~TaskSimulator() {
	Destroy();
}