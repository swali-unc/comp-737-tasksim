#include "OpenFileView.hpp"

#include <stdexcept>
#include <Windows.h>

#include "ViewManager.hpp"
#include "TitleView.hpp"
#include "SimulationState.hpp"

// These three includes will go away in future
#include "SimulationView.hpp"
#include "NonPreemptiveEDF.hpp"
#include "TaskSimulator.hpp"

using namespace sf;
using std::string;
using std::runtime_error;
using std::thread;
using std::exception;

void* OpenFileThread(void* data);

bool OpenFileView::Render(RenderWindow& window, Vector2f mouse, bool clicked) {
	window.draw(*waitText.getCachedSprite());

	LockObject();
	if(fileOpenStatus == NO_FILE_PICKED) {
		UnlockObject();
		ViewManager::Instance()->queueClear();
		ViewManager::Instance()->queueView(new TitleView());
		return false;
	}
	else if(fileOpenStatus == FILE_PICKED) {
		UnlockObject(); // If a file was picked, this shouldn't be contended anymore

		ViewManager::Instance()->queueClear();
		ProblemSet* problem;
		try {
			problem = new ProblemSet(filepath);
			auto ss = SimulationState::Instance();
			ss->setProblem(problem);
			printf("Problem set\n");
			// TODO: next view should be to pick an algorithm
			ss->setScheduler(new NonPreemptiveEDF());
			printf("Scheduler set\n");

			// TODO: simulation in progress view
			ss->setSimulation(new TaskSimulator());
			printf("Simulation set\n");
			ss->getSimulator()->LoadProblem();
			printf("Simulation set\n");
			while(ss->getSimulator()->getTime() < ss->getProblem()->getScheduleLength()) {
				printf("Simulating %f\n", ss->getSimulator()->getTime());
				ss->getSimulator()->Simulate();
			}

			ViewManager::Instance()->queueView(new SimulationView());

		}
		catch(exception e) {
			fprintf(stderr, "Could not load problem: %s", e.what());
			ViewManager::Instance()->queueView(new TitleView());
		}

		return false;
	}
	return true;
}

OpenFileView::OpenFileView() : ViewObject(), ThreadSpinLockedObject(),
	waitText("Waiting for problem XML to be selected..",Color::Black,24) {
	fileOpenStatus = OpenFileViewStatus::UNKNOWN;

	fileOpenThread = new thread(OpenFileThread, (void*)this);
	fileOpenThread->detach();
}

OpenFileView::~OpenFileView() {
	delete fileOpenThread;
}

void* OpenFileThread(void* data) {
	OpenFileView* view = (OpenFileView*)data;
	view->LockObject();
	view->fileOpenStatus = IN_DIALOG;
	view->UnlockObject();

	OPENFILENAMEA ofn = { 0 };
	char szFile[_MAX_PATH] = { 0 };

	// Initialize remaining fields of OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "XML\0*.XML\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileNameA(&ofn)) {
		// use ofn.lpstrFile here
		view->LockObject();
		strcpy_s(view->filepath, ofn.lpstrFile);
		view->fileOpenStatus = FILE_PICKED;
		view->UnlockObject();
	}
	else {
		view->LockObject();
		view->fileOpenStatus = NO_FILE_PICKED;
		view->UnlockObject();
	}

	return nullptr;
}