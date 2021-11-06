#include "SimulationView.hpp"

#include "SimulationState.hpp"
#include <vector>
#include <utility>
#include <string>

#include "JobStatusEvent.hpp"

using namespace sf;
using std::vector;
using std::pair;
using std::make_pair;
using std::string;

bool SimulationView::Render(RenderWindow& window, Vector2f mouse, bool clicked) {
	for(auto i = 0u; i < procCount; ++i) {
		window.draw(*timelines[i]->getCachedSprite());
	}

	return true;
}

void SimulationView::createTimeline(unsigned int proc) {
	if(timelines[proc]) delete timelines[proc];
	ScheduleSprite* sprite;

	vector<pair<double, string>> releases;
	vector<JobExecution> executions;
	vector<pair<double, string>> completions;
	vector<pair<double, string>> deadlines;
	
	auto schedule = SimulationState::Instance()->getSimulator()->getSchedule();
	for(auto& i : schedule) {
		switch(i->getType()) {
		case ScheduleEventType::ReleaseEvent:
			releases.push_back(
				make_pair<double, string>(i->getStart(),((JobReleaseEvent*)i)->getJob()->createLabel())
			);
			break;
		case ScheduleEventType::ExecutionEvent:
			executions.push_back(*(JobExecution*)i);
			break;
		case ScheduleEventType::FinishEvent:
			completions.push_back(
				make_pair<double, string>(i->getStart(), ((JobFinishEvent*)i)->getJob()->createLabel())
			);
			break;
		case ScheduleEventType::DeadlineEvent:
			deadlines.push_back(
				make_pair<double, string>(i->getStart(), ((JobDeadlineEvent*)i)->getJob()->createLabel())
			);
			break;
		};
	}

	sprite = new ScheduleSprite(timeStart, timeEnd, SimulationState::Instance()->getProblem()->getTimelineInterval(),
		executions.data(), executions.size(),
		releases.data(), releases.size(),
		deadlines.data(), deadlines.size(),
		completions.data(), completions.size());

	timelines[proc] = sprite;
	sprite->doMouseoverRegistrations(0, 0);
}

SimulationView::SimulationView() : ViewObject() {
	simulationInProgress = true;
	procCount = SimulationState::Instance()->getProblem()->getProcessorCount();

	timelines = new ScheduleSprite * [procCount];
	for(auto i = 0u; i < procCount; ++i)
		timelines[i] = nullptr;

	timeStart = 0;
	timeEnd = 40 * SimulationState::Instance()->getProblem()->getTimelineInterval();

	for(auto i = 0u; i < procCount; ++i)
		createTimeline(i);
}

SimulationView::~SimulationView() {
	for(auto i = 0u; i < procCount; ++i) {
		if(timelines[i])
			delete timelines[i];
	}

	delete[] timelines;
}