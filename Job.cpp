#include "Job.hpp"

using std::string;
using std::vector;

JobExecution Job::executeJob(double executionTime, double execStart) {
	JobExecution jExec(*this, execStart, executionTime);

	for (auto& i : resourceAccess) {
		// Did not we already pass this resource?
		if (currentTime <= i.start + i.duration) {
			// Is our current time in the interval, or will our execution put us in the interval?
			if (currentTime >= i.start || currentTime + executionTime >= i.start) {
				// If our execution is also going to pull us out of this resource, then
				// we don't really need to go out of our way to add it
				if (currentTime + executionTime < i.start + i.duration) {
					// Access the resource for a partial amount of time
					//jExec.addResourceAccess(i.start - currentTime, i.start + i.duration - (currentTime + executionTime), i.resourceName);
					jExec.addResourceAccess(i.start - currentTime, currentTime + executionTime - i.start, i.resourceName);
					accessResource(i.resourceName);
				}
				else {
					// Access the resource for the full duration
					jExec.addResourceAccess(i.start - currentTime, i.duration, i.resourceName);
				}
			}
		}

		// We need to detect resource intervals ending, so we must
		// be in an interval first
		if (currentTime >= i.start // Past the start of the resource
			&& currentTime <= i.start + i.duration // Before the end of the resource
			&& currentTime + executionTime > i.start + i.duration // our execution puts us beyond the end of resource
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

bool Job::getTimeOfNextResource(double& resourceStartDelta, std::vector<std::string>& resourceNames) const {
	double startTime = -1;

	for (auto& i : resourceAccess) {
		if (currentTime <= i.start) {
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
	resourceStartDelta = startTime;
	return true;
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
}