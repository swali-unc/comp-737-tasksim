#pragma once

#include "Scheduler.hpp"

#include <Windows.h>
#include <string>

#define TASKSIM_VERSION 1ul
#define MIN_REQUIRED_VERSION 1ul

typedef void (*OnJobRelease)(double, void*);
typedef void (*OnJobFinish)(double, void*, unsigned int);
typedef void (*OnJobDeadline)(double, void*);
typedef void (*OnJobSliceFinish)(double, void*, unsigned int);
typedef void (*OnJobResourceRequest)(double, void*, const char*, unsigned int);
typedef void (*OnJobResourceFinish)(double, void*, const char*, unsigned int);
typedef void (*OnTimer)(double, void*);
typedef void (*OnIdle)(double, unsigned int);
typedef unsigned long (*IdentifyAsScheduler)(unsigned long);

class DLLScheduler : public Scheduler
{
public:
	DLLScheduler(std::string filename);
	~DLLScheduler();

	virtual void onJobRelease(double time, Job* job) override;
	virtual void onJobFinish(double time, Job* job, unsigned int proc) override;
	virtual void onJobDeadline(double time, Job* job) override;
	virtual void onJobSliceFinish(double time, Job* job, unsigned int proc) override;
	virtual void onResourceRequest(double time, Job* job, std::string resourceName, unsigned int proc) override;
	virtual void onResourceFinish(double time, Job* job, std::string resourceName, unsigned int proc) override;
	virtual void onTimer(double time, void* timerdata) override;
	virtual void onIdle(double time, unsigned int proc) override;
private:
	HMODULE library;

	// Reason for the generic names is because we just call these immediately
	// if not null, so no point in lengthy names

	OnJobRelease ojr;
	OnJobFinish ojf;
	OnJobDeadline ojd;
	OnJobSliceFinish ojsf;
	OnJobResourceRequest ojrr;
	OnJobResourceFinish ojrf;
	OnTimer ot;
	OnIdle oi;
};

