#pragma once

#include <string>
#include "Job.hpp"

class Scheduler
{
public:
	Scheduler() {}
	virtual ~Scheduler() {}

	virtual void onJobRelease(double time, Job* job) = 0;
	virtual void onJobFinish(double time, Job* job) = 0;
	virtual void onJobDeadline(double time, Job* job) = 0;
	virtual void onResourceRequest(double time, Job* job, std::string resourceName) = 0;
	virtual void onResourceFinish(double time, Job* job, std::string resourceName) = 0;
	virtual void onTimer(double time, void* timerdata) = 0;
	virtual void onIdle(double time) = 0;
private:
};

