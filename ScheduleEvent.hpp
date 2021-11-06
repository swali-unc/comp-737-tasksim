#pragma once

enum class ScheduleEventType {
	ExecutionEvent,
	CommentEvent,
	ReleaseEvent,
	DeadlineEvent,
	FinishEvent,
};

class ScheduleEvent
{
public:
	ScheduleEvent(double start, double duration = 0) noexcept {
		this->start = start;
		this->duration = duration;
	}
	virtual ~ScheduleEvent() {}

	virtual ScheduleEventType getType() const = 0;

	inline double getStart() const { return start; }
	inline double getDuration() const { return duration; }
	inline void setDuration(double newDuration) { duration = newDuration; }
private:
	double start;
	double duration;
};

class ScheduleEventComparator {
public:
	bool operator()(const ScheduleEvent* lhs, ScheduleEvent* rhs) const {
		return lhs->getStart() > rhs->getStart();
	}
};