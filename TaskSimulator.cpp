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

auto constexpr DOUBLE_ROUND = 0.000001;

bool TaskSimulator::Simulate() {
	auto sched = SimulationState::Instance()->getScheduler();
	if (!sched) {
		logScheduleError("No scheduler loaded",0);
		return false;
	}

	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();

	ScheduleEvent* nextEvent;
	double nextEventTime;
	if (!upcomingEventQueue.size()) {
		nextEvent = nullptr;
		nextEventTime = SimulationState::Instance()->getProblem()->getScheduleLength();
	}
	else {
		nextEvent = upcomingEventQueue.top();
		nextEventTime = nextEvent->getStart();
	}

	enum _nextEventType {
		UNKNOWN, COMPLETION, RESOURCEACCESS, RESOURCERELEASE, TIMEFINISH
	};
	_nextEventType nextEventType = UNKNOWN;
	vector<string> eventResources;

	auto nextEventProc = 0u;
	for(auto i = 0u; i < procCount; ++i) {
		if(currentJobOnProc[i]) {
			auto remainingCost = currentJobOnProc[i]->getRemainingCost();

			// slice finish is the last thing to report
			if(currentJobStart[i] + currentDuration[i] <= nextEventTime) {
				nextEventType = TIMEFINISH;
				nextEventTime = currentJobStart[i] + currentDuration[i];
				nextEventProc = i;
			}

			// completion is the last thing to report
			if(currentJobStart[i] + remainingCost <= nextEventTime) {
				nextEventType = COMPLETION;
				nextEventTime = currentJobStart[i] + remainingCost;
				nextEventProc = i;
			}

			auto currentTimeDelta = time - currentJobStart[i];
			double resourceDelta;
			vector<string> resources;

			if(currentJobOnProc[i]->getTimeOfNextResource(resourceDelta, resources, currentTimeDelta)) {
				if(time + resourceDelta <= nextEventTime) {
					eventResources = resources;
					nextEventTime = time + resourceDelta;
					nextEventType = RESOURCEACCESS;
					nextEventProc = i;
				}
			}

			if(currentJobOnProc[i]->getTimeOfNextResourceRelease(resourceDelta, resources, currentTimeDelta)) {
				if(time + resourceDelta <= nextEventTime) {
					eventResources = resources;
					nextEventTime = time + resourceDelta;
					nextEventType = RESOURCERELEASE;
					nextEventProc = i;
				}
			}

		}
	}


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

	// We have a job event and not a regular event
	if(nextEventType != UNKNOWN) {
		auto job = currentJobOnProc[nextEventProc];

		// Job finish event
		if(nextEventType == COMPLETION) {
			schedules[nextEventProc].push_back(job->executeJob(time - currentJobStart[nextEventProc], currentJobStart[nextEventProc]));

			schedules[nextEventProc].push_back(new JobFinishEvent(time, *job));
			currentJobs.erase(std::find(currentJobs.begin(), currentJobs.end(), job));
			currentJobOnProc[nextEventProc] = nullptr;
			sched->onJobFinish(time, job, nextEventProc);
			//delete job;
			jobGarbage.push_back(job);
			return true;
		}
		else if(nextEventType == RESOURCEACCESS) {
			for(auto& i : eventResources)
				sched->onResourceRequest(time, job, i, nextEventProc);
			return true;
		}
		else if(nextEventType == RESOURCERELEASE) {
			for(auto& i : eventResources)
				sched->onResourceFinish(time, job, i, nextEventProc);
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
			for(auto i = 0u; i < procCount; ++i) {
				if(currentJobOnProc[i])
					StopExecutingCurrentJob(i);
			}
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
		unsigned int proc;
		double timeExecuted;
		if (latestProc < 0) {
			proc = 0;
			timeExecuted = 0;
		}
		else {
			proc = (unsigned int)latestProc;
			timeExecuted = (time - currentJobStart[proc]);
		}
		//auto proc = latestProc < 0 ? 0u : (unsigned int)latestProc;

		schedules[proc].push_back(deadlineEvent);

		// Did we miss a deadline?
		if (deadlineEvent->getJob()->getRemainingCost() > DOUBLE_ROUND) {
			logScheduleError(stringprintf("Deadline missed for %s (%s remaining)",
				deadlineEvent->getJob()->createLabel().c_str(), to_string_trim(
					deadlineEvent->getJob()->getRemainingCost() - (timeExecuted)
				).c_str() ), proc);
		}

		sched->onJobDeadline(time, deadlineEvent->getJob());
		return true;
	}

	logScheduleError(stringprintf("Error, unknown event type: %d", eventType), 0);
	return false;
}

bool TaskSimulator::NeedsSimulation() {
	if(time < SimulationState::Instance()->getProblem()->getScheduleLength())
		return true;

	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();
	for(auto i = 0u; i < procCount; ++i) {
		if(currentJobOnProc[i]) return true;
	}

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
	currentJobOnProc[proc] = job;
	currentJobStart[proc] = time;
	currentDuration[proc] = duration;
	job->setAssignedProcessor(proc);

	//printf("%s assigned to proc %u for %f @ %f\n", job->createLabel().c_str(), proc, duration, time);
	return true;
}

bool TaskSimulator::StopExecutingCurrentJob(unsigned int proc) {
	if (!currentJobOnProc[proc]) {
		logScheduleError("Attempted to stop executing current job on processor, but no job currently is executing.",proc);
		return false;
	}

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

bool TaskSimulator::isJobUsingResource(Job* job, string resource) {
	// Is this job currently being executed?
	auto currentProc = job->getLatestAssignedProcessor();
	if(currentProc < 0) return false;
	if(currentJobOnProc[currentProc] == job) {
		// We're being executed, so need to figure out where
		// we are in the execution to determine the resource
		// usage
		return job->isResourceAccessed(resource, time - currentJobStart[currentProc]);
	}

	// This job is not being executed, so determine
	// if it is using said resource
	return job->isResourceAccessed(resource);
}

void TaskSimulator::logScheduleError(string errorText, unsigned int proc) {
	schedules[proc].push_back(new CommentEvent(time, errorText));
	SimulationState::Instance()->logError(time, errorText);
	fprintf(stderr, "[Proc %u] Scheduler error reported: %s\n", proc, errorText.c_str());
}

void TaskSimulator::Reset() {
	time = 0;
	auto procCount = SimulationState::Instance()->getProblem()->getProcessorCount();
	Destroy();

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

	for (auto& i : jobGarbage)
		delete i;

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