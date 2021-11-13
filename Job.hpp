/* Job
 * 
 * A job is defined by the tuple (release,absolute deadline,relative deadline,cost).
 * We keep two additional parameters, and that is the index and originating task (if any).
 * If this job came from a task, then we can retrieve the original task, otherwise it
 * is an aperiodic.
 */
#pragma once

#include "SchedulerExports.h"
#include "Task.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>

#include "ResourceAccessParameter.h"
#include "JobExecution.hpp"

class JobExecution;

class DLLCLASSMODE Job
{
public:
	Job(Task& t, int index = 0) noexcept;
	Job(Task& t, double releaseTime, int index) noexcept;
	Job(double releaseTime, double absDeadline, double relativeDeadline, double cost, int index = 0) noexcept;

	JobExecution* executeJob(double executionTime, double execStart = 0);
	inline void accessResource(std::string resourceName) { heldResources[resourceName] = true; }
	void releaseResource(std::string resourceName);
	bool getTimeOfNextResource(double& resourceStartDelta, std::vector<std::string>& resourceNames, double delta = 0) const;
	bool getTimeOfNextResourceRelease(double& resourceReleaseDelta, std::vector<std::string>& resourceNames, double delta = 0) const;
	bool isResourceAccessed(std::string resourceName, double delta = 0) const;
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
	inline int getLatestAssignedProcessor() const { return task ? task->getLatestAssignedProcessor() : lastProcessor; }
	inline void setAssignedProcessor(unsigned int proc) { lastProcessor = (int)proc; if(task) task->setLatestAssignedProcessor(proc); }
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
	int lastProcessor;

	std::vector<ResourceAccessParameter> resourceAccess;
	std::map<std::string, bool> heldResources;
};

class JobReleaseComparator {
public:
	bool operator()(const Job* lhs, Job* rhs) const {
		return lhs->getReleaseTime() > rhs->getReleaseTime();
	}
};