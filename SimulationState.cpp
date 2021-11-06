#include "SimulationState.hpp"
#include "MouseoverRegistration.hpp"

auto constexpr MAX_TICKS_PER_WINDOW = 40;

void SimulationState::setProblem(ProblemSet* problem) {
	if (this->problem) delete this->problem;
	this->problem = problem;
	errorList.clear();
	MouseoverRegistration::Instance()->clearAll();
	this->currentIntervalStart = 0;
	this->currentIntervalEnd = problem->getTimelineInterval() * MAX_TICKS_PER_WINDOW;
	if (this->currentIntervalEnd > problem->getScheduleLength())
		this->currentIntervalEnd = problem->getScheduleLength() - this->currentIntervalStart;
}

void SimulationState::setSimulation(TaskSimulator* sim) {
	if (taskSim) delete taskSim;
	taskSim = sim;
}

void SimulationState::setScheduler(Scheduler* sched) {
	if (scheduler) delete scheduler;
	scheduler = sched;
}

SimulationState::SimulationState() {
	problem = nullptr;
	taskSim = nullptr;
	scheduler = nullptr;
}

SimulationState::~SimulationState() {
	if (problem) delete problem;
	if (taskSim) delete taskSim;
	if (scheduler) delete scheduler;
}

SimulationState* SimulationState::_instance = nullptr;
SimulationState* SimulationState::Instance() {
	if (!_instance)
		_instance = new SimulationState();
	return _instance;
}