#pragma once

#include "Job.hpp"

class JobExecution
{
public:
	JobExecution(Job& job, double start, double duration) noexcept;
	JobExecution() noexcept {} // Dummy constructor

	inline double getStart() const { return start; }
	inline double getDuration() const { return duration; }
	inline Job* getJob() const { return job; }
private:
	double start;
	double duration;
	Job* job;
};

