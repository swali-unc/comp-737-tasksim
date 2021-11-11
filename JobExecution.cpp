#include "JobExecution.hpp"

JobExecution::JobExecution(Job& job, double start, double duration, double sliceDelta) noexcept
	: ScheduleEvent(start,duration) {
	this->job = &job;
	this->sliceDelta = sliceDelta;
}

JobExecution::JobExecution(const JobExecution& copyObj) noexcept
	: ScheduleEvent(copyObj.getStart(),copyObj.getDuration()) {
	job = copyObj.job;

	resourceAccess = copyObj.resourceAccess;
}
