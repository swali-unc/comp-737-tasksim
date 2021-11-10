#include "SimulationView.hpp"

#include "SimulationState.hpp"
#include <vector>
#include <utility>
#include <string>

#include "JobStatusEvent.hpp"
#include "MouseoverRegistration.hpp"

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
	
	auto schedule = SimulationState::Instance()->getSimulator()->getSchedule(proc);
	for(auto& i : schedule) {
		if(i->getStart() > timeEnd)
			continue;

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
		executions.data(), (unsigned int)executions.size(),
		releases.data(), (unsigned int)releases.size(),
		deadlines.data(), (unsigned int)deadlines.size(),
		completions.data(), (unsigned int)completions.size());

	timelines[proc] = sprite;
	sprite->getCachedSprite()->setPosition(100.f, (float)(100 * (proc + 1)));
	sprite->doMouseoverRegistrations(100.f, (float)(100 * (proc+1)));
}

SimulationView::SimulationView() : ViewObject() {
	simulationInProgress = true;
	procCount = SimulationState::Instance()->getProblem()->getProcessorCount();

	timelines = new ScheduleSprite * [procCount];
	for(auto i = 0u; i < procCount; ++i)
		timelines[i] = nullptr;

	timeStart = 0;
	timeEnd = 40 * SimulationState::Instance()->getProblem()->getTimelineInterval();

	MouseoverRegistration::Instance()->clearAll();
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