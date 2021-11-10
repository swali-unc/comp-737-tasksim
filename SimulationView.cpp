#include "SimulationView.hpp"

#include "SimulationState.hpp"
#include <vector>
#include <utility>
#include <string>

#include "JobStatusEvent.hpp"
#include "CommentEvent.hpp"
#include "MouseoverRegistration.hpp"

using namespace sf;
using std::vector;
using std::pair;
using std::make_pair;
using std::string;
using std::bind;

auto constexpr TIME_BUTTON_WIDTH = 30u;
auto constexpr TIME_BUTTON_HEIGHT = 30u;
auto constexpr INCREMENTS_TO_SHOW = 40.f;

bool SimulationView::Render(RenderWindow& window, Vector2f mouse, bool clicked) {
	for(auto i = 0u; i < procCount; ++i) {
		window.draw(*timelines[i]->getCachedSprite());
	}

	// Also render buttons
	return ButtonView::Render(window, mouse, clicked);
}

void SimulationView::timeBackward() {
	auto interval = SimulationState::Instance()->getProblem()->getTimelineInterval();
	auto oneChunk = interval * INCREMENTS_TO_SHOW;
	auto timeDelta = oneChunk / 2.f;
	auto nextStart = timeStart - timeDelta;
	auto nextEnd = timeEnd - timeDelta;
	if(nextStart < 0) {
		nextStart = 0;
		nextEnd = oneChunk;
	}
	timeStart = nextStart;
	timeEnd = nextEnd;
	CreateRenders();
}

void SimulationView::timeForward() {
	auto interval = SimulationState::Instance()->getProblem()->getTimelineInterval();
	auto oneChunk = interval * INCREMENTS_TO_SHOW;
	auto timeDelta = oneChunk / 2.f;
	auto nextStart = timeStart + timeDelta;
	auto nextEnd = timeEnd + timeDelta;
	auto maxTime = SimulationState::Instance()->getProblem()->getScheduleLength();
	if(nextEnd > maxTime) {
		nextStart = maxTime - oneChunk;
		nextEnd = maxTime;
	}
	timeStart = nextStart;
	timeEnd = nextEnd;
	CreateRenders();
}

void SimulationView::CreateRenders() {
	// Clear the mouseover registrations
	MouseoverRegistration::Instance()->clearView(this);
	for(auto i = 0u; i < procCount; ++i)
		createTimeline(i);
}

void SimulationView::createTimeline(unsigned int proc) {
	if(timelines[proc]) delete timelines[proc];

	vector<pair<double, string>> releases;
	vector<JobExecution> executions;
	vector<pair<double, string>> completions;
	vector<pair<double, string>> deadlines;
	vector<pair<double, string>> comments;
	
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
		case ScheduleEventType::CommentEvent:
			comments.push_back(
				make_pair<double, string>(i->getStart(), ((CommentEvent*)i)->getComment())
			);
			break;
		};
	}

	auto sprite = new ScheduleSprite(timeStart, timeEnd, SimulationState::Instance()->getProblem()->getTimelineInterval(), this,
		executions.data(), (unsigned int)executions.size(),
		releases.data(), (unsigned int)releases.size(),
		deadlines.data(), (unsigned int)deadlines.size(),
		completions.data(), (unsigned int)completions.size(),
		comments.data(), (unsigned int)comments.size()
	);

	timelines[proc] = sprite;
	sprite->getCachedSprite()->setPosition(100.f, 50.f + (float)(100 * proc));
	sprite->doMouseoverRegistrations(100.f, 50.f + (float)(100 * proc));
}

SimulationView::SimulationView() : ButtonView() {
	simulationInProgress = true;
	procCount = SimulationState::Instance()->getProblem()->getProcessorCount();

	timelines = new ScheduleSprite * [procCount];
	for(auto i = 0u; i < procCount; ++i)
		timelines[i] = nullptr;

	timeStart = 0;
	timeEnd = INCREMENTS_TO_SHOW * SimulationState::Instance()->getProblem()->getTimelineInterval();

	//MouseoverRegistration::Instance()->clearAll();
	for(auto i = 0u; i < procCount; ++i)
		createTimeline(i);

	timeForwardBtn = new UIButton(">", bind(&SimulationView::timeForward, this), TIME_BUTTON_WIDTH, TIME_BUTTON_HEIGHT);
	timeBackwardBtn = new UIButton("<", bind(&SimulationView::timeBackward, this), TIME_BUTTON_WIDTH, TIME_BUTTON_HEIGHT);
	timeForwardBtn->setButtonPosition(25.f + TIME_BUTTON_WIDTH + 5.f, 30.f);
	timeBackwardBtn->setButtonPosition(25.f, 30.f);
	registerButton(timeForwardBtn);
	registerButton(timeBackwardBtn);
}

SimulationView::~SimulationView() {
	for(auto i = 0u; i < procCount; ++i) {
		if(timelines[i])
			delete timelines[i];
	}

	delete[] timelines;

	if(timeForwardBtn) delete timeForwardBtn;
	if(timeBackwardBtn) delete timeBackwardBtn;
	MouseoverRegistration::Instance()->clearView(this);
}