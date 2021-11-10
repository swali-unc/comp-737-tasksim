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

	void LoadProblem(ProblemSet* problem = nullptr);
	void Reset();
	
	bool Schedule(Job* job, double duration, unsigned int proc);
	bool StopExecutingCurrentJob(unsigned int proc);
	bool IsIdle(unsigned int proc) const { return currentJobOnProc[proc] == nullptr; }
	Job* GetJobOnProcessor(unsigned int proc) const { return currentJobOnProc[proc]; }

	bool Simulate();
	std::vector<Job*> getCurrentJobs() const { return currentJobs; }
	std::vector<ScheduleEvent*> getSchedule(unsigned int proc) const { return schedules[proc]; }

	double getTime() const { return time; }
	void logScheduleError(std::string errorText, unsigned int proc);

	void* registerTimer(double delta, void* callbackData);
	double getRemainingCostOfJob(Job* job);
private:
	std::vector<std::pair<double, void*>*> timers;

	std::priority_queue<ScheduleEvent*, std::vector<ScheduleEvent*>, ScheduleEventComparator> upcomingEventQueue;
	std::priority_queue<Job*, std::vector<Job*>, JobReleaseComparator> upcomingJobReleases;

	std::vector<ScheduleEvent*>* schedules;
	std::vector<Job*> currentJobs;
	double time;
	Job** currentJobOnProc;
	double* currentJobStart;
	double* currentDuration;

	void Destroy();
};

