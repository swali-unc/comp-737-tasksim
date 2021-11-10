#pragma once

#include <string>
#include "Job.hpp"

class Scheduler
{
public:
	Scheduler() {}
	virtual ~Scheduler() {}

	virtual void onJobRelease(double time, Job* job) = 0;
	virtual void onJobFinish(double time, Job* job, unsigned int proc) = 0;
	virtual void onJobDeadline(double time, Job* job) = 0;
	virtual void onJobSliceFinish(double time, Job* job, unsigned int proc) = 0;
	virtual void onResourceRequest(double time, Job* job, std::string resourceName, unsigned int proc) = 0;
	virtual void onResourceFinish(double time, Job* job, std::string resourceName,unsigned int proc) = 0;
	virtual void onTimer(double time, void* callbackData, void* timerPointer) = 0;
	virtual void onIdle(double time, unsigned int proc) = 0;
private:
};

