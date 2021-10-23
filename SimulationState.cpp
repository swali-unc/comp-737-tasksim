#include "SimulationState.hpp"
#include "MouseoverRegistration.hpp"

void SimulationState::setProblem(ProblemSet* problem) {
	if (this->problem) delete this->problem;
	this->problem = problem;
	MouseoverRegistration::Instance()->clearAll();
}

SimulationState::SimulationState() {
	problem = nullptr;
}

SimulationState::~SimulationState() {
	if (problem) delete problem;
}

SimulationState* SimulationState::_instance = nullptr;
SimulationState* SimulationState::Instance() {
	if (!_instance)
		_instance = new SimulationState();
	return _instance;
}