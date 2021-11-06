#include "JobExecution.hpp"

JobExecution::JobExecution(Job& job, double start, double duration) noexcept
	: ScheduleEvent(start,duration) {
	this->job = &job;
}

JobExecution::JobExecution(const JobExecution& copyObj) noexcept
	: ScheduleEvent(copyObj.getStart(),copyObj.getDuration()) {
	job = copyObj.job;

	resourceAccess = copyObj.resourceAccess;
}
