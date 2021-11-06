#pragma once

#include "ScheduleEvent.hpp"
#include <string>

class CommentEvent : public ScheduleEvent
{
public:
	CommentEvent(double time, std::string comment) noexcept : ScheduleEvent(time, 0) { this->comment = comment; }

	virtual ScheduleEventType getType() const { return ScheduleEventType::CommentEvent; }
	inline std::string getComment() const { return comment; }
private:
	std::string comment;
};

