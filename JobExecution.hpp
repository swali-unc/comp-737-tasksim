#pragma once

#include "Job.hpp"
#include "ResourceAccessParameter.h"

#include <vector>
#include <string>

class Job;

class JobExecution
{
public:
	JobExecution(Job& job, double start, double duration) noexcept;
	JobExecution() noexcept {} // Dummy constructor

	inline double getStart() const { return start; }
	inline double getDuration() const { return duration; }
	inline Job* getJob() const { return job; }
	inline void addResourceAccess(double time, double duration, std::string name) { resourceAccess.push_back({ name,time,duration }); }
	inline std::vector<ResourceAccessParameter>& getResourceParameters() { return resourceAccess; }
private:
	double start;
	double duration;
	Job* job;

	std::vector<ResourceAccessParameter> resourceAccess;
};

