#include "OpenFileView.hpp"

#include <stdexcept>
#include <Windows.h>

#include "ViewManager.hpp"
#include "TitleView.hpp"
#include "SimulationState.hpp"

// These three includes will go away in future
#include "SimulationView.hpp"
#include "DLLScheduler.hpp"
#include "TaskSimulator.hpp"

using namespace sf;
using std::string;
using std::runtime_error;
using std::thread;
using std::exception;

auto constexpr PROBLEM_DIR = "Problems\\";
auto constexpr SCHEDULER_DIR = "Schedulers\\";

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
		try {
			auto ss = SimulationState::Instance();

			// If we don't have a problem set, then we just loaded that
			if(!ss->getProblem()) {
				// And the next step will be to load the scheduler
				ss->setProblem(new ProblemSet(filepath));
				ViewManager::Instance()->queueClear();
				ViewManager::Instance()->queueView(
					new OpenFileView("Please select an algorithm DLL",
						"Scheduling Algorithm DLL\0*.DLL\0All\0*.*\0\0")
				);
				return false;
			}

			// If we got this far, then we have a problem loaded, but now need a scheduler
			ss->setScheduler(new DLLScheduler(filepath));

			// Now that the scheduler is loaded
			ss->setSimulation(new TaskSimulator());
			ss->getSimulator()->LoadProblem();

			while( ss->getSimulator()->NeedsSimulation() ) {
				ss->getSimulator()->Simulate();
			}

			ViewManager::Instance()->queueClear();
			ViewManager::Instance()->queueView(new SimulationView());
		}
		catch(exception e) {
			fprintf(stderr, "Could not load problem: %s", e.what());
			ViewManager::Instance()->queueClear();
			ViewManager::Instance()->queueView(new TitleView());

			// Set our state back from anything we may have loaded
			auto ss = SimulationState::Instance();
			if(ss->getProblem()) {
				delete ss->getProblem();
				ss->setProblem(nullptr);
			}
			if(ss->getScheduler()) {
				delete ss->getScheduler();
				ss->setScheduler(nullptr);
			}
		}

		return false;
	}
	UnlockObject();
	return true;
}

OpenFileView::OpenFileView(string text, const char* filetypes) : ViewObject(), ThreadSpinLockedObject(),
	waitText(text,Color::Black,24) {
	this->filetypes = filetypes;
	fileOpenStatus = OpenFileViewStatus::UNKNOWN;

	fileOpenThread = new thread(OpenFileThread, (void*)this);
	fileOpenThread->detach();
}

OpenFileView::~OpenFileView() {
	delete fileOpenThread;
}

void* OpenFileThread(void* data) {
	OpenFileView* view = (OpenFileView*)data;
	const char* filetypes;
	const char* defaultDir = nullptr;
	view->LockObject();
	view->fileOpenStatus = IN_DIALOG;
	filetypes = view->filetypes;
	if(!SimulationState::Instance()->getProblem())
		defaultDir = PROBLEM_DIR;
	else if(!SimulationState::Instance()->getScheduler())
		defaultDir = SCHEDULER_DIR;
	view->UnlockObject();

	OPENFILENAMEA ofn = { 0 };
	char szFile[_MAX_PATH] = { 0 };
	ZeroMemory(&ofn,sizeof(ofn));
	ZeroMemory(szFile, sizeof(szFile));

	// Initialize remaining fields of OPENFILENAME structure
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filetypes; //"XML\0*.XML\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = defaultDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

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