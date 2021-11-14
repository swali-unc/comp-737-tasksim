#include "LoadingSimView.hpp"

#include <stdexcept>
#include "SimulationState.hpp"
#include "ViewManager.hpp"
#include "TitleView.hpp"
#include "SimulationView.hpp"

#include "Utility.hpp"
#include "TextSprite.hpp"

using std::runtime_error;
using std::bind;
using std::thread;
using DigitalHaze::ThreadSpinLockedObject;
using namespace sf;

auto constexpr SIMFRAMES_PER_LOOP = 5;
auto constexpr PROGRESS_FONT_SIZE = 16;
auto constexpr PROGRESS_BAR_WIDTH = 800.f;
auto constexpr PROGRESS_BAR_HEIGHT = 50.f;
#define PROGRESS_FONT_COLOR Color::Black
#define PROGRESS_BAR_COLOR Color::Cyan

void* SimulationThread(void* loadingSimView);

bool LoadingSimView::Render(RenderWindow& window, Vector2f mouse, bool clicked) {
	LockObject();
	auto currentState = simulationStatus;
	UnlockObject();
	auto ss = SimulationState::Instance();

	if (currentState == (int)SimProgress::SIMULATING) {
		auto ctime = ss->getSimulator()->getTime();

		// Output progress bar
		RectangleShape bar(Vector2f((float)(PROGRESS_BAR_WIDTH * (ctime / totalTime)), (float)PROGRESS_BAR_HEIGHT));
		bar.setFillColor(PROGRESS_BAR_COLOR);
		bar.setPosition(640.f - (PROGRESS_BAR_WIDTH / 2.f), 200.f);
		window.draw(bar);

		// Output time status
		TextSprite spr(stringprintf("%s/%s",
			to_string_trim(ctime).c_str(),
			to_string_trim(totalTime).c_str()
			), PROGRESS_FONT_COLOR, PROGRESS_FONT_SIZE);
		auto sprite = spr.getCachedSprite();
		sprite->setPosition(640.f, 200.f);
		window.draw(*sprite);
	}
	else if (currentState == (int)SimProgress::SIMULATION_SUCCESS_FINISH) {
		// Move on to the simulation view
		auto vm = ViewManager::Instance();
		vm->queueClear();
		vm->queueView(new SimulationView());
		return false;
	}
	else {
		// Something bad happened
		switch (currentState) {
		case (int)SimProgress::SIMULATION_CANCEL_FINISH:
		case (int)SimProgress::SIMULATION_ERROR_FINISH:
			ViewManager::Instance()->queueClear();
			ViewManager::Instance()->queueView(new TitleView());
			return false;
		};
	}

	return ButtonView::Render(window, mouse, clicked);
}

void LoadingSimView::CancelSimBtnCallback() {
	LockObject();
	if( simulationStatus == (int)SimProgress::SIMULATING )
		simulationStatus = (int)SimProgress::SIMULATION_CANCEL;
	UnlockObject();
}

LoadingSimView::LoadingSimView() : ButtonView(), ThreadSpinLockedObject() {
	cancelBtn = nullptr;

	auto ss = SimulationState::Instance();
	if (!ss->getProblem())
		throw runtime_error("There is no problem loaded, cannot load simulation");
	if (!ss->getScheduler())
		throw runtime_error("There is no scheduling algorithm loaded, cannot load simulator");

	cancelBtn = new UIButton("Cancel", bind(&LoadingSimView::CancelSimBtnCallback, this), 200, 90);
	cancelBtn->setButtonPosition(640.f, 360.f);
	registerButton(cancelBtn);

	totalTime = ss->getProblem()->getScheduleLength();

	simulationStatus = (int)SimProgress::SIMULATING;
	simulationThread = new thread(SimulationThread, (void*)this);
	simulationThread->detach();
}

LoadingSimView::~LoadingSimView() {
	if (cancelBtn) delete cancelBtn;
	if (simulationThread) delete simulationThread;
}

void* SimulationThread(void* loadingSimView) {
	auto simView = (LoadingSimView*)loadingSimView;
	auto ss = SimulationState::Instance();
	auto sim = ss->getSimulator();

	sim = new TaskSimulator();
	sim->LoadProblem(ss->getProblem());
	ss->setSimulation(sim);

	for (;;) {
		simView->LockObject();
		if (simView->simulationStatus == (int)SimProgress::SIMULATION_CANCEL) {
			simView->UnlockObject();
			break;
		}
		simView->UnlockObject();

		for (auto i = 0; i < SIMFRAMES_PER_LOOP; ++i) {
			if (!sim->NeedsSimulation()) {
				simView->LockObject();
				simView->simulationStatus = (int)SimProgress::SIMULATION_SUCCESS;
				simView->UnlockObject();
				break;
			}
			if (!sim->Simulate()) {
				simView->LockObject();
				simView->simulationStatus = (int)SimProgress::SIMULATION_ERROR;
				simView->UnlockObject();
				break;
			}
		}

		simView->LockObject();
		if (simView->simulationStatus != (int)SimProgress::SIMULATING) {
			simView->UnlockObject();
			break;
		}
		simView->UnlockObject();
	}

	simView->LockObject();
	// Unload the scheduling algorithm
	ss->setScheduler(nullptr);

	// In error events, unload the problem as well
	if (simView->simulationStatus == (int)SimProgress::SIMULATION_CANCEL) {
		ss->setProblem(nullptr);
		simView->simulationStatus = (int)SimProgress::SIMULATION_CANCEL_FINISH;
	}
	else if(simView->simulationStatus == (int)SimProgress::SIMULATION_ERROR) {
		ss->setProblem(nullptr);
		simView->simulationStatus = (int)SimProgress::SIMULATION_ERROR_FINISH;
	}
	else if (simView->simulationStatus == (int)SimProgress::SIMULATION_SUCCESS) {
		// No need to do anything
		simView->simulationStatus = (int)SimProgress::SIMULATION_SUCCESS_FINISH;
	}
	else {
		// This shouldn't happen
		fprintf(stderr, "Simulation Thread terminated with no clear status indicator (%d)\n", simView->simulationStatus);
	}

	simView->UnlockObject();
	return nullptr;
}