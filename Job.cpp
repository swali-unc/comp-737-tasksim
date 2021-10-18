#include "Job.hpp"

void Job::executeJob(double executionTime) {
	remainingCost -= executionTime;
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
}

Job::Job(double releaseTime, double absDeadline, double relativeDeadline, double cost, int index) noexcept {
	this->release = releaseTime;
	this->adeadline = absDeadline;
	this->rdeadline = relativeDeadline;
	this->cost = cost;
	this->index = index;
	this->task = nullptr;
	this->remainingCost = cost;
}