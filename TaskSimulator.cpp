#include "TaskSimulator.hpp"

#include <algorithm>
#include "JobStatusEvent.hpp"
#include "CommentEvent.hpp"
#include "Utility.hpp"
#include "SimulationState.hpp"

using std::find;
using std::string;

bool TaskSimulator::Simulate() {
	auto sched = SimulationState::Instance()->getScheduler();
	if (!sched) {
		logScheduleError("No scheduler loaded");
		return false;
	}

	auto nextEvent = upcomingEventQueue.top();
	auto nextEventTime = nextEvent ? nextEvent->getStart() : SimulationState::Instance()->getProblem()->getTimelineInterval();
	//bool nextEventIsCompletion = false, nextEventIsResourceAccess = false, nextEventIsResourceRelease = false;
	enum _nextEventType {
		UNKNOWN, COMPLETION, RESOURCEACCESS, RESOURCERELEASE, TIMEFINISH
	};
	_nextEventType nextEventType;
	string resource;
	//printf("Next event time: %f\n", nextEventTime);
	if (currentJob) {
		auto remainingCost = currentJob->getRemainingCost();

		// Will we hit a resource access/release?
		string resourceName, resourceReleaseName;
		auto timeTillResource = currentJob->getNextResourceAccess(resourceName);
		auto timeTillResourceRelease = currentJob->getNextResourceRelease(resourceReleaseName);
		if(timeTillResource > 0 && time + timeTillResource < nextEventTime) {
			nextEventTime = time + timeTillResource;
			nextEventType = RESOURCEACCESS;
			resource = resourceName;
		}

		if(timeTillResourceRelease > 0 && time + timeTillResourceRelease < nextEventTime) {
			nextEventType = RESOURCERELEASE;
			resource = resourceName;
			nextEventTime = time + timeTillResourceRelease;
		}

		if(currentJobStart + currentDuration < nextEventTime) {
			nextEventType = TIMEFINISH;
			nextEventTime = currentJobStart + currentDuration;
		}
		
		if (currentJobStart + remainingCost < nextEventTime ) {
			// Our current job will finish executing before the next event
			// so the real next event is a job completion
			nextEventType = COMPLETION;
			nextEventTime = currentJobStart + remainingCost;
		}
	}

	auto elapsedTime = nextEventTime - time;
	auto startTime = time;
	time = nextEventTime;
	if (currentJob) {
		auto job = currentJob;

		// Job finish event
		if (nextEventType == COMPLETION) {
			schedule.push_back(job->executeJob(time - currentJobStart, currentJobStart));

			schedule.push_back(new JobFinishEvent(time, *job));
			currentJobs.erase(std::find(currentJobs.begin(), currentJobs.end(), job));
			currentJob = nullptr;
			sched->onJobFinish(time, job);
			return true;
		}
		else if(nextEventType == RESOURCEACCESS) {
			sched->onResourceRequest(time, job, resource);
			return true;
		}
		else if(nextEventType == RESOURCERELEASE) {
			sched->onResourceFinish(time, job, resource);
			return true;
		}
		else if(nextEventType == TIMEFINISH) {
			sched->onJobSliceFinish(time, job);
			StopExecutingCurrentJob();
			return true;
		}
	}

	// There was no next event, we're at the end?
	if (!nextEvent) {
		sched->onIdle(time);
		return true;
	}

	// Remove this event
	upcomingEventQueue.pop();

	// Is it a job release?
	auto eventType = nextEvent->getType();
	if (eventType == ScheduleEventType::ReleaseEvent) {
		JobReleaseEvent* releaseEvent = static_cast<JobReleaseEvent*>(nextEvent);
		schedule.push_back(releaseEvent);
		currentJobs.push_back(releaseEvent->getJob());
		sched->onJobRelease(time, releaseEvent->getJob());
		return true;
	}

	// Did we get a deadline?
	if (eventType == ScheduleEventType::DeadlineEvent) {
		JobDeadlineEvent* deadlineEvent = static_cast<JobDeadlineEvent*>(nextEvent);
		schedule.push_back(deadlineEvent);

		// Did we miss a deadline?
		if (deadlineEvent->getJob()->getRemainingCost() > 0) {
			logScheduleError(stringprintf("Deadline missed for %s (%f)",
				deadlineEvent->getJob()->createLabel().c_str(), time));
		}

		sched->onJobDeadline(time, deadlineEvent->getJob());
		return true;
	}

	logScheduleError(stringprintf("Error, unknown event type: %d", eventType));
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

bool TaskSimulator::Schedule(Job* job, double duration) {
	if (currentJob) {
		if (currentJob == job) {
			// If it's the same job, then no need to do anything
			//double currentDuration = currentJob->getDuration();
			//currentJob->setDuration((currentDuration - (time - currentJobStart)) + duration);

			return true;
		}

		// End the current job execution at this time
		StopExecutingCurrentJob();
	}

	// Can't schedule this job if it isn't in our current job list
	if (find(currentJobs.begin(), currentJobs.end(), job) == currentJobs.end()) {
		logScheduleError(stringprintf(
			"Simulation Error: Algorithm tried to schedule job %s that has either not released yet or is unknown",
			job->createLabel().c_str()
		));
		return false;
	}

	// schedule our new job
	//currentJob = new JobExecution(*job, time, duration);
	currentJob = job;
	currentJobStart = time;
	currentDuration = duration;
	//schedule.push_back(currentJob);
	return true;
}

bool TaskSimulator::StopExecutingCurrentJob() {
	if (!currentJob) {
		logScheduleError("Attempted to stop executing current job, but no job currently is executing.");
		return false;
	}

	//currentJob->setDuration(time - currentJobStart);
	if(currentJobStart < time) {
		schedule.push_back(currentJob->executeJob(time - currentJobStart, currentJobStart));
	}

	currentJob = nullptr;
	return true;
}

void TaskSimulator::logScheduleError(string errorText) {
	schedule.push_back(new CommentEvent(time, errorText));
	SimulationState::Instance()->logError(time, errorText);
	fprintf(stderr, "Scheduler error reported: %s\n", errorText.c_str());
}

void TaskSimulator::Reset() {
	time = 0;
	currentJob = nullptr;

	while (upcomingEventQueue.size()) {
		delete upcomingEventQueue.top();
		upcomingEventQueue.pop();
	}

	while (currentJobs.size()) {
		delete currentJobs[currentJobs.size() - 1];
		currentJobs.pop_back();
	}

	while (upcomingJobReleases.size()) {
		delete upcomingJobReleases.top();
		upcomingJobReleases.pop();
	}

	while (schedule.size()) {
		delete schedule[schedule.size() - 1];
		schedule.pop_back();
	}
}

TaskSimulator::TaskSimulator() noexcept {
	Reset();
}

TaskSimulator::~TaskSimulator() {
	Reset();
}

/*
bool TaskSimulator::ScheduleJob(Job* job, double start, double duration) {
	// Bad time given? Can't go back in time
	if (time < start) {
		string errorText = stringprintf("Simulation Error: Algorithm tried to schedule job %s in the past for [%s,%s)",
			job->createLabel().c_str(),
			to_string_trim(start).c_str(), to_string_trim(duration).c_str()
		);
		schedule.push_back(new CommentEvent(start, errorText));
		SimulationState::Instance()->logError(start, errorText);
		return false;
	}

	// Is this even a job that we have active?
	if (find(currentJobs.begin(), currentJobs.end(), job) == currentJobs.end()) {
		string errorText = stringprintf("Simulation Error: Algorithm tried to schedule job %s that has either not released yet or is unknown",
			job->createLabel().c_str()
		);
		schedule.push_back(new CommentEvent(start,errorText));
		SimulationState::Instance()->logError(start, errorText);
		return false;
	}

	// Is anything else executing in this time slot?
	for (auto& i : schedule) {
		if (i->getType() == ScheduleEventType::ExecutionEvent) {
			// Check if:
			//  (1) our start point is inside of another execution slice
			//  (2) our end point is inside of another execution slice
			//  (3) we fully encompass another job
			auto checkIfInSlice = [](double check, double start, double duration) {
				// True if we're above the start of this slice and below the end of this slice
				return start <= check && start + duration > check;
			};
			if (checkIfInSlice(start,i->getStart(),i->getDuration())
				|| checkIfInSlice(start + duration, i->getStart(), i->getDuration())
				|| (start <= i->getStart() && start + duration >= i->getStart() + i->getDuration())) {
				string errorText = stringprintf("Simulation Error: Algorithm tried to schedule job %s at [%s,%s) but it conflicts with job %s",
					job->createLabel().c_str(),
					to_string_trim(start).c_str(), to_string_trim(duration).c_str(),
					((JobExecution*)i)->getJob()->createLabel().c_str()
				);
				schedule.push_back(new CommentEvent(start,errorText));
				SimulationState::Instance()->logError(start,errorText);
				return false;
			}
		}
	}

	// Alright, schedule this sucker
	schedule.push_back(new JobExecution(*job, start, duration));
	return true;
}*/