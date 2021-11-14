#include "SimulationView.hpp"

#include <vector>
#include <utility>
#include <string>

#include "SimulationState.hpp"
#include "ViewManager.hpp"
#include "JobStatusEvent.hpp"
#include "CommentEvent.hpp"
#include "MouseoverRegistration.hpp"
#include "Utility.hpp"
#include "OpenFileView.hpp"

using namespace sf;
using std::vector;
using std::pair;
using std::make_pair;
using std::string;
using std::bind;

auto constexpr TIME_BUTTON_WIDTH = 30u;
auto constexpr TIME_BUTTON_HEIGHT = 30u;
auto constexpr INCREMENTS_TO_SHOW = 40.f;
auto constexpr TIME_LABEL_SIZE = 12u;
auto constexpr PROC_LABEL_SIZE = 15u;
auto constexpr ERROR_BUTTON_WIDTH = 150u;
auto constexpr ERROR_BUTTON_HEIGHT = 30u;
auto constexpr MAX_LETTERS_IN_ERROR_BUTTON = 20;
auto constexpr CHANGE_PROBLEM_WIDTH = 100;
#define TIME_LABEL_COLOR Color::Black
#define PROC_LABEL_COLOR Color::Black

bool SimulationView::Render(RenderWindow& window, Vector2f mouse, bool clicked) {
	for(auto i = 0u; i < procCount; ++i) {
		window.draw(*timelines[i]->getCachedSprite());
		window.draw(*processorNames[i]->getCachedSprite());
	}

	// Also render buttons
	window.draw(*currentTimeSprite->getCachedSprite());
	return ButtonView::Render(window, mouse, clicked);
}

void SimulationView::timeBackward() {
	ErrorButtonCallback(timeStart);
}

void SimulationView::timeForward() {
	ErrorButtonCallback(timeEnd);
}

void SimulationView::ErrorButtonCallback(double time) {
	auto interval = SimulationState::Instance()->getProblem()->getTimelineInterval();
	auto duration = SimulationState::Instance()->getProblem()->getScheduleLength();
	auto oneChunk = interval * INCREMENTS_TO_SHOW;
	auto timeDelta = oneChunk / 2.f;
	auto nextStart = time - timeDelta;
	if (nextStart < 0) {
		time = timeDelta;
		nextStart = 0;
	}
	auto nextEnd = time + timeDelta;
	if(nextEnd > duration && nextEnd > oneChunk)
		nextEnd = duration;

	if(timeStart == nextStart)
		return;

	timeStart = nextStart;
	timeEnd = nextEnd;
	CreateRenders();
}

void SimulationView::CreateRenders() {
	// Clear the mouseover registrations
	MouseoverRegistration::Instance()->clearView(this);
	for(auto i = 0u; i < procCount; ++i)
		createTimeline(i);
	createTimeSprite();
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
		if(simulationInProgress && i->getType() == ScheduleEventType::CommentEvent) {
			// In this scenario, we need to know about all errors.
			string comment = ((CommentEvent*)i)->getComment();
			if(comment.length() > MAX_LETTERS_IN_ERROR_BUTTON)
				comment = comment.substr(0, MAX_LETTERS_IN_ERROR_BUTTON - 3) + "...";
			errors.push_back(
				make_pair<double,UIButton*>(
					i->getStart(),
					new UIButton(comment,
						bind(&SimulationView::ErrorButtonCallback,this,i->getStart()),
						ERROR_BUTTON_WIDTH, ERROR_BUTTON_HEIGHT)) );
			auto btn = errors.at(errors.size() - 1).second;
			btn->setButtonPosition(1280 - ERROR_BUTTON_WIDTH, 10.f + (errors.size() - 1) * (ERROR_BUTTON_HEIGHT + 5.f));
			registerButton(btn);
		}

		if(i->getStart() > timeEnd || i->getStart() + i->getDuration() < timeStart)
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

void SimulationView::createTimeSprite() {
	if(currentTimeSprite) delete currentTimeSprite;
	currentTimeSprite = new TextSprite(stringprintf("[%s,%s]",
		to_string_trim(timeStart).c_str(),
		to_string_trim(timeEnd).c_str()
	), TIME_LABEL_COLOR, TIME_LABEL_SIZE);

	currentTimeSprite->getCachedSprite()->setPosition(25.f, 30.f + TIME_BUTTON_HEIGHT + 5.f);
}

void SimulationView::NewProblemButton() {
	auto ss = SimulationState::Instance();
	auto vm = ViewManager::Instance();

	ss->setProblem(nullptr);
	ss->setScheduler(nullptr);

	vm->queueClear();
	vm->queueView(new OpenFileView());
}

void SimulationView::NewSchedulerButton() {
	auto ss = SimulationState::Instance();
	auto vm = ViewManager::Instance();

	ss->setScheduler(nullptr);

	vm->queueClear();
	vm->queueView(new OpenFileView());
}

SimulationView::SimulationView() : ButtonView() {
	currentTimeSprite = nullptr;
	processorNames = nullptr;
	simulationInProgress = true;
	procCount = SimulationState::Instance()->getProblem()->getProcessorCount();

	timeStart = 0;
	timeEnd = INCREMENTS_TO_SHOW * SimulationState::Instance()->getProblem()->getTimelineInterval();

	timelines = new ScheduleSprite * [procCount];
	processorNames = new TextSprite * [procCount];
	for(auto i = 0u; i < procCount; ++i) {
		timelines[i] = nullptr;
		createTimeline(i);
		processorNames[i] = new TextSprite(stringprintf("Processor %u", i), PROC_LABEL_COLOR, PROC_LABEL_SIZE);
		processorNames[i]->getCachedSprite()->setPosition(5.f, 100.f + (float)(100 * i) + PROC_LABEL_SIZE);
	}

	timeForwardBtn = new UIButton(">", bind(&SimulationView::timeForward, this), TIME_BUTTON_WIDTH, TIME_BUTTON_HEIGHT);
	timeBackwardBtn = new UIButton("<", bind(&SimulationView::timeBackward, this), TIME_BUTTON_WIDTH, TIME_BUTTON_HEIGHT);
	timeForwardBtn->setButtonPosition(25.f + TIME_BUTTON_WIDTH + 5.f, 30.f);
	timeBackwardBtn->setButtonPosition(25.f, 30.f);
	registerButton(timeForwardBtn);
	registerButton(timeBackwardBtn);
	createTimeSprite();

	newProblemBtn = new UIButton("New Problem", bind(&SimulationView::NewProblemButton, this), CHANGE_PROBLEM_WIDTH, TIME_BUTTON_HEIGHT);
	newSchedulerBtn = new UIButton("New Scheduler", bind(&SimulationView::NewSchedulerButton, this), CHANGE_PROBLEM_WIDTH, TIME_BUTTON_HEIGHT);
	newProblemBtn->setButtonPosition(125.f, 10.f);
	newSchedulerBtn->setButtonPosition(130.f + CHANGE_PROBLEM_WIDTH, 10.f);
	registerButton(newProblemBtn);
	registerButton(newSchedulerBtn);

	simulationInProgress = false;
}

SimulationView::~SimulationView() {
	for(auto i = 0u; i < procCount; ++i) {
		if(timelines && timelines[i])
			delete timelines[i];
		if(processorNames && processorNames[i])
			delete processorNames[i];
	}

	if( timelines )
		delete[] timelines;
	if( processorNames )
		delete[] processorNames;
	for(auto& i : errors)
		delete i.second;

	if(timeForwardBtn) delete timeForwardBtn;
	if(timeBackwardBtn) delete timeBackwardBtn;
	if(currentTimeSprite) delete currentTimeSprite;
	if (newProblemBtn) delete newProblemBtn;
	if (newSchedulerBtn) delete newSchedulerBtn;
	MouseoverRegistration::Instance()->clearView(this);
}