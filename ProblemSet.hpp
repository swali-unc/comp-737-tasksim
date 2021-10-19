#pragma once

#include "Task.hpp"
#include "Job.hpp"
#include "tinyxml2.h"

#include <string>

class ProblemSet
{
public:
	ProblemSet(std::string filename);
	~ProblemSet();

	inline Task** getTaskSet() const { return taskSet; }
	inline Task** getTaskSet(int& nTasks) const { nTasks = numTasks; return taskSet; }
	inline int getTaskCount() const { return numTasks; }
	inline Job** getAperiodics() const { return aperiodicJobs; }
	inline Job** getAperiodics(int& nJobs) const { nJobs = numAperiodics; return aperiodicJobs; }
	inline int getAperiodicCount() const { return numAperiodics; }
private:
	int numTasks;
	int numAperiodics;
	Task** taskSet;
	Job** aperiodicJobs;

	void initializeTaskSet(tinyxml2::XMLElement* tasks);
	void initializeAperiodics(tinyxml2::XMLElement* jobs);
};

