#pragma once

#include "Job.hpp"
#include "ScheduleEvent.hpp"
#include "ResourceAccessParameter.h"

#include <vector>
#include <string>

class Job;

class JobExecution : public ScheduleEvent
{
public:
	JobExecution(Job& job, double start, double duration) noexcept;
	JobExecution() noexcept : ScheduleEvent(0,0) {} // Dummy constructor

	inline double getStart() const { return start; }
	inline double getDuration() const { return duration; }
	inline double setDuration(double duration) { this->duration = duration; }
	inline Job* getJob() const { return job; }
	inline void addResourceAccess(double time, double duration, std::string name) { resourceAccess.push_back({ name,time,duration }); }
	inline std::vector<ResourceAccessParameter>& getResourceParameters() { return resourceAccess; }
	virtual ScheduleEventType getType() const { return ScheduleEventType::ExecutionEvent; }
private:
	double start;
	double duration;
	Job* job;

	std::vector<ResourceAccessParameter> resourceAccess;
};

