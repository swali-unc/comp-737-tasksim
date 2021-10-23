#pragma once

#include "Task.hpp"
#include "Job.hpp"
#include "tinyxml2.h"

#include <string>

class ProblemSet
{
public:
	ProblemSet() noexcept { }
	ProblemSet(std::string filename);
	~ProblemSet();

	inline Task** getTaskSet() const { return taskSet; }
	inline Task** getTaskSet(int& nTasks) const { nTasks = numTasks; return taskSet; }
	inline int getTaskCount() const { return numTasks; }
	inline Job** getAperiodics() const { return aperiodicJobs; }
	inline Job** getAperiodics(int& nJobs) const { nJobs = numAperiodics; return aperiodicJobs; }
	inline int getAperiodicCount() const { return numAperiodics; }
	inline double getScheduleLength() const { return scheduleLength; }
	inline unsigned int getProcessorCount() const { return processorCount; }
	inline double getTimelineInterval() const { return timelineInterval; }

	void loadProblem(std::string filename);
private:
	int numTasks;
	int numAperiodics;
	Task** taskSet;
	Job** aperiodicJobs;
	double scheduleLength;
	unsigned int processorCount;
	double timelineInterval;

	void initializeTaskSet(tinyxml2::XMLElement* tasks);
	void initializeAperiodics(tinyxml2::XMLElement* jobs);
	void initializeProblem(tinyxml2::XMLElement* problem);
};

