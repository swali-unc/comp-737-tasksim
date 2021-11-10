#pragma once

#include "Scheduler.hpp"
#include <queue>
#include <vector>

class NonPreemptiveEDF : public Scheduler
{
public:
	NonPreemptiveEDF();
	virtual ~NonPreemptiveEDF();

	virtual void onJobRelease(double time, Job* job) override;
	virtual void onJobFinish(double time, Job* job, unsigned int proc) override;
	virtual void onJobDeadline(double time, Job* job) override;
	virtual void onJobSliceFinish(double time, Job* job, unsigned int proc) override;
	virtual void onResourceRequest(double time, Job* job, std::string resourceName, unsigned int proc) override;
	virtual void onResourceFinish(double time, Job* job, std::string resourceName, unsigned int proc) override;
	virtual void onTimer(double time, void* timerdata, void* timerpointer) override;
	virtual void onIdle(double time, unsigned int proc) override;
private:
};