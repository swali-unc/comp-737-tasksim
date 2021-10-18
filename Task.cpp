#include "Task.hpp"

Task::Task(double period, double cost, int index) noexcept
	: Task(0, period, cost, period, index) {
}

Task::Task(double period, double cost, double relativeDeadline, int index) noexcept
	: Task(0, period, cost, relativeDeadline, index) {
}

Task::Task(double phase, double period, double cost, double relativeDeadline, int index) noexcept {
	this->phase = phase;
	this->period = period;
	this->cost = cost;
	this->rdeadline = relativeDeadline;
	this->index = index;
}