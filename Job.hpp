/* Job
 * 
 * A job is defined by the tuple (release,absolute deadline,relative deadline,cost).
 * We keep two additional parameters, and that is the index and originating task (if any).
 * If this job came from a task, then we can retrieve the original task, otherwise it
 * is an aperiodic.
 */
#pragma once

#include "Task.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>

#include "ResourceAccessParameter.h"
#include "JobExecution.hpp"

class JobExecution;

class Job
{
public:
	Job(Task& t, int index = 0) noexcept;
	Job(Task& t, double releaseTime, int index) noexcept;
	Job(double releaseTime, double absDeadline, double relativeDeadline, double cost, int index = 0) noexcept;

	JobExecution* executeJob(double executionTime, double execStart = 0);
	inline void accessResource(std::string resourceName) { heldResources[resourceName] = true; }
	void releaseResource(std::string resourceName);
	bool getTimeOfNextResource(double& resourceStartDelta, std::vector<std::string>& resourceNames) const;
	bool getTimeOfNextResourceRelease(double& resourceReleaseDelta, std::vector<std::string>& resourceNames) const;
	std::string createLabel() const;

	inline double getReleaseTime() const { return release; }
	inline double getAbsoluteDeadline() const { return adeadline; }
	inline double getRelativeDeadline() const { return rdeadline; }
	inline double getCost() const { return cost; }
	inline Task* getRelatedTask() const { return task; }
	inline int getIndex() const { return index; }
	inline double getRemainingCost() const { return remainingCost; }
	inline bool hasFinished() const { return remainingCost <= 0; }
	inline sf::Color getColor() const { return color; }
	inline void setColor(sf::Color color) { this->color = color; }
	inline void addResourceAccess(double time, double duration, std::string name) { resourceAccess.push_back({name,time,duration}); }
	inline std::vector<ResourceAccessParameter>& getResourceParameters() { return resourceAccess; }

	double getNextResourceAccess(std::string& resourceName) const;
	double getNextResourceRelease(std::string& resourceName) const;
private:
	double release;
	double adeadline;
	double rdeadline;
	double cost;
	Task* task;
	int index;
	double remainingCost;
	double currentTime;
	sf::Color color;

	std::vector<ResourceAccessParameter> resourceAccess;
	std::map<std::string, bool> heldResources;
};

class JobReleaseComparator {
public:
	bool operator()(const Job* lhs, Job* rhs) const {
		return lhs->getReleaseTime() > rhs->getReleaseTime();
	}
};