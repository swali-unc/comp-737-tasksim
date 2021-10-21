#include "JobExecution.hpp"

JobExecution::JobExecution(Job& job, double start, double duration) noexcept {
	this->job = &job;
	this->start = start;
	this->duration = duration;
}