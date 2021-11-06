#pragma once

#include "ProblemSet.hpp"
#include "TaskSimulator.hpp"
#include "Scheduler.hpp"

#include <vector>
#include <utility>
#include <string>

auto constexpr WINDOW_WIDTH = 1280u;
auto constexpr WINDOW_HEIGHT = 720u;

class SimulationState
{
public:
	SimulationState();
	~SimulationState();

	inline TaskSimulator* getSimulator() const { return taskSim; }
	inline ProblemSet* getProblem() const { return problem; };
	inline Scheduler* getScheduler() const { return scheduler; }
	inline std::vector<std::pair<double, std::string>>* getErrors() { return &errorList; }
	// Do not pass locals, this ProblemSet pointer will be memory managed and free'd
	void setProblem(ProblemSet* problem);
	// Do not pass locals, this Simulator pointer will be memory managed and free'd
	void setSimulation(TaskSimulator* sim);
	// Do not pass locals, this Scheduler pointer will be memory managed and free'd
	void setScheduler(Scheduler* schd);

	inline void logError(double time, std::string errorText) { errorList.push_back(std::make_pair(time, errorText)); }

	static SimulationState* Instance();
private:
	static SimulationState* _instance;
	ProblemSet* problem;
	TaskSimulator* taskSim;
	Scheduler* scheduler;
	double currentIntervalStart;
	double currentIntervalEnd;
	std::vector<std::pair<double, std::string>> errorList;
};

