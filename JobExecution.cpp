#include "JobExecution.hpp"

JobExecution::JobExecution(Job& job, double start, double duration) noexcept
	: ScheduleEvent(start,duration) {
	this->job = &job;
	this->start = start;
	this->duration = duration;
}