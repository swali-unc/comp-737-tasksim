#pragma once

#include "SchedulerExports.h"
#include "Job.hpp"
#include "ScheduleEvent.hpp"
#include "ResourceAccessParameter.h"

#include <vector>
#include <string>

class Job;

class DLLCLASSMODE JobExecution : public ScheduleEvent
{
public:
	JobExecution(Job& job, double start, double duration, double sliceDelta) noexcept;
	JobExecution() noexcept : ScheduleEvent(0,0) {} // Dummy constructor
	JobExecution(const JobExecution& copyObj) noexcept;
	virtual ~JobExecution() { }

	inline Job* getJob() const { return job; }
	inline void addResourceAccess(double time, double duration, std::string name) { resourceAccess.push_back({ name,time,duration }); }
	inline std::vector<ResourceAccessParameter>& getResourceParameters() { return resourceAccess; }
	virtual ScheduleEventType getType() const { return ScheduleEventType::ExecutionEvent; }
	inline double getSliceDelta() const { return sliceDelta; }
private:
	Job* job;
	double sliceDelta;

	std::vector<ResourceAccessParameter> resourceAccess;
};

