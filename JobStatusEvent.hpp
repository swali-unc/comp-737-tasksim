#pragma once

#include "ScheduleEvent.hpp"

#include <utility>
#include "Job.hpp"

class JobReleaseEvent : public ScheduleEvent
{
public:
	JobReleaseEvent(Job& job) noexcept : ScheduleEvent(job.getReleaseTime(), 0) { this->job = &job; }

	virtual ScheduleEventType getType() const { return ScheduleEventType::ReleaseEvent; }
	inline Job* getJob() const { return job; }
	std::pair<double, std::string> createLabel() const { return std::make_pair(job->getReleaseTime(), job->createLabel()); }
private:
	Job* job;
};

class JobDeadlineEvent : public ScheduleEvent
{
public:
	JobDeadlineEvent(Job& job) noexcept : ScheduleEvent(job.getAbsoluteDeadline(), 0) { this->job = &job; }

	virtual ScheduleEventType getType() const { return ScheduleEventType::DeadlineEvent; }
	inline Job* getJob() const { return job; }
	std::pair<double, std::string> createLabel() const { return std::make_pair(job->getAbsoluteDeadline(), job->createLabel()); }
private:
	Job* job;
};

class JobFinishEvent : public ScheduleEvent
{
public:
	JobFinishEvent(double time, Job& job) noexcept : ScheduleEvent(time, 0) { this->job = &job; this->finishTime = time; }

	virtual ScheduleEventType getType() const { return ScheduleEventType::FinishEvent; }
	inline Job* getJob() const { return job; }
	std::pair<double, std::string> createLabel() const { return std::make_pair(finishTime, job->createLabel()); }
private:
	Job* job;
	double finishTime;
};