#pragma once

#include "JobExecution.hpp"
#include "ScheduleEvent.hpp"
#include "ProblemSet.hpp"
#include "Scheduler.hpp"

#include <queue>
#include <vector>
#include <string>

class TaskSimulator
{
public:
	TaskSimulator() noexcept;
	~TaskSimulator();

	void LoadProblem(ProblemSet* problem);
	void Reset();
	
	bool Schedule(Job* job, double duration);
	//bool ScheduleJob(Job* job, double start, double duration);
	bool StopExecutingCurrentJob();
	bool IsIdle() const { return currentJob == nullptr; }

	bool Simulate();
	std::vector<Job*> getCurrentJobs() const { return currentJobs; } //C++11 will move, not copy
private:
	void logScheduleError(std::string errorText);

	std::priority_queue<ScheduleEvent*, std::vector<ScheduleEvent*>, ScheduleEventComparator> upcomingEventQueue;
	std::priority_queue<Job*, std::vector<Job*>, JobReleaseComparator> upcomingJobReleases;

	std::vector<ScheduleEvent*> schedule;
	std::vector<Job*> currentJobs;
	double time;
	JobExecution* currentJob;
	double currentJobStart;
};

