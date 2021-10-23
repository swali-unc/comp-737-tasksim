#pragma once

#include "ProblemSet.hpp"

class SimulationState
{
public:
	SimulationState();
	~SimulationState();

	inline ProblemSet* getProblem() const { return problem; };
	// Do not pass locals, this ProblemSet will be memory managed
	void setProblem(ProblemSet* problem);

	static SimulationState* Instance();
private:
	static SimulationState* _instance;
	ProblemSet* problem;
};

