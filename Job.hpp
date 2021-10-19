/* Job
 * 
 * A job is defined by the tuple (release,absolute deadline,relative deadline,cost).
 * We keep two additional parameters, and that is the index and originating task (if any).
 * If this job came from a task, then we can retrieve the original task, otherwise it
 * is an aperiodic.
 */
#pragma once

#include "Task.hpp"

class Job
{
public:
	Job(Task& t, int index = 0) noexcept;
	Job(Task& t, double releaseTime, int index) noexcept;
	Job(double releaseTime, double absDeadline, double relativeDeadline, double cost, int index = 0) noexcept;

	void executeJob(double executionTime);

	inline double getReleaseTime() const { return release; }
	inline double getAbsoluteDeadline() const { return adeadline; }
	inline double getRelativeDeadline() const { return rdeadline; }
	inline double getCost() const { return cost; }
	inline Task* getRelatedTask() const { return task; }
	inline int getIndex() const { return index; }
	inline double getRemainingCost() const { return remainingCost; }
	inline bool hasFinished() const { return remainingCost <= 0; }
private:
	double release;
	double adeadline;
	double rdeadline;
	double cost;
	Task* task;
	int index;
	double remainingCost;
};

