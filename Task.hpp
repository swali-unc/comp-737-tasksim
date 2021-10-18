#pragma once

class Task
{
public:
	Task(double phase, double period, double cost, double relativeDeadline, int index) noexcept;
	Task(double period, double cost, double relativeDeadline, int index) noexcept;
	Task(double period, double cost, int index) noexcept;

	inline double getPhase() const { return phase; }
	inline double getPeriod() const { return period; }
	inline double getCost() const { return cost; }
	inline double getRelativeDeadline() const { return rdeadline; }
	inline int getIndex() const { return index; }
private:
	double phase;
	double period;
	double cost;
	double rdeadline;
	int index;
};