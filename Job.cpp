#include "Job.hpp"

using std::string;
using std::vector;

JobExecution* Job::executeJob(double executionTime, double execStart) {
	JobExecution* jExec = new JobExecution(*this, execStart, executionTime, currentTime);

	for (auto& i : resourceAccess) {
		// Did not we already pass this resource?
		if (currentTime < i.start + i.duration) {
			// Is our current time in the interval, or will our execution put us in the interval?
			if (currentTime >= i.start || currentTime + executionTime >= i.start) {
				//printf("%s Times: %f + %f [%f,%f)\n", createLabel().c_str(), currentTime, executionTime, i.start, i.start + i.duration);

				// Three interchangeable scenarios
				// Scenario 1: I am already in a resource, and now that I am executing, I need only a resource slice
				// Scenario 2: I am not in a resource, so if the resource is in in my execution, put it in
				// Scenario 3: I am leaving a resource
				
				auto resourceStartPoint = i.start;
				if(currentTime <= i.start) {
					// resource hasn't started yet
					auto timeInResource = executionTime - (i.start - currentTime);
					auto resourceDuration = timeInResource > i.duration ? i.duration : timeInResource;
					jExec->addResourceAccess(i.start - currentTime, resourceDuration, i.resourceName);
					if(resourceDuration < i.duration)
						accessResource(i.resourceName);
				}
				else {
					// We are already in the resource
					auto resourceAmountUsed = currentTime - i.start;
					auto resourceDuration = i.duration - resourceAmountUsed;
					jExec->addResourceAccess(0, resourceDuration, i.resourceName);
				}
			}
		}

		// We need to detect resource intervals ending, so we must
		// be in an interval first
		if (currentTime >= i.start // Past the start of the resource
			&& currentTime < i.start + i.duration // Before the end of the resource
			&& currentTime + executionTime >= i.start + i.duration // our execution puts us beyond the end of resource
			)
			releaseResource(i.resourceName);
	}

	remainingCost -= executionTime;
	currentTime += executionTime;

	return jExec;
}

void Job::releaseResource(string resourceName) {
	auto i = heldResources.find(resourceName);
	if (i == heldResources.end())
		return;
	heldResources.erase(i);
}

bool Job::getTimeOfNextResource(double& resourceStartDelta, std::vector<std::string>& resourceNames, double delta) const {
	double startTime = -1;

	for (auto& i : resourceAccess) {
		if (currentTime + delta < i.start) {
			// Find something earlier than our earliest?
			if (i.start < startTime || startTime == -1) {
				// Clear our list because this resource is earlier
				resourceNames.clear();
				resourceNames.push_back(i.resourceName);
				startTime = i.start;
			}
			else if (i.start == startTime) // Add because of shared time
				resourceNames.push_back(i.resourceName);
		}
	}

	if (startTime == -1)
		return false;
	resourceStartDelta = startTime - (currentTime+delta);
	return true;
}

bool Job::getTimeOfNextResourceRelease(double& resourceReleaseDelta, vector<string>& resourceNames, double delta) const {
	double releaseTime = -1;

	for (auto& i : resourceAccess) {
		// Are we currently in this resource segment?
		if (currentTime + delta >= i.start && currentTime + delta < i.start + i.duration) {
			if (i.start + i.duration < releaseTime || releaseTime == -1) {
				resourceNames.clear();
				resourceNames.push_back(i.resourceName);
				releaseTime = i.start + i.duration;
			}
			else if (i.start + i.duration == releaseTime)
				resourceNames.push_back(i.resourceName);
		}
	}

	if (releaseTime == -1)
		return false;
	resourceReleaseDelta = releaseTime - (currentTime + delta);
	return true;
}

bool Job::isResourceAccessed(string name, double delta) const {
	// Loop through our resources and see if the specified time period
	// is within a resource access
	auto timeCheck = currentTime + delta;
	for(auto& i : resourceAccess) {
		if(name == i.resourceName) {
			if(i.start < timeCheck && timeCheck < i.start + i.duration)
				return true;
		}
	}

	return false;
}

std::string Job::createLabel() const {
	std::string str = "J";

	// Do we have a related task?
	if (task)
		str += std::to_string(task->getIndex()) + ",";
	str += std::to_string(index);
	return str;
}

Job::Job(Task& t, int index) noexcept
	: Job(t, 0, index) {
}

Job::Job(Task& t, double releaseTime, int index) noexcept
	: Job(releaseTime,
		releaseTime + t.getRelativeDeadline(),
		t.getRelativeDeadline(),
		t.getCost(),
		index) {
	this->task = &t;
	this->color = t.getColor();
	this->resourceAccess = t.getResourceParameters();
	this->lastProcessor = t.getLatestAssignedProcessor();
}

Job::Job(double releaseTime, double absDeadline, double relativeDeadline, double cost, int index) noexcept {
	this->release = releaseTime;
	this->adeadline = absDeadline;
	this->rdeadline = relativeDeadline;
	this->cost = cost;
	this->index = index;
	this->task = nullptr;
	this->remainingCost = cost;
	this->currentTime = 0;
	this->color = sf::Color::White;
	this->lastProcessor = -1;
}