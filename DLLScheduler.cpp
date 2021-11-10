#include "DLLScheduler.hpp"

#include "Utility.hpp"
#include <stdexcept>

using std::string;
using std::invalid_argument;

#define ProcAddress(fname) (fname)GetProcAddress(library,#fname)
#define NAME(x) #x

DLLScheduler::DLLScheduler(string filename) : Scheduler() {
	library = LoadLibraryA(filename.c_str());
	if(library == NULL)
		throw invalid_argument(stringprintf("Error %ul: could not load library %s",GetLastError()));
	auto signature = ProcAddress(IdentifyAsScheduler);
	if(signature == NULL) {
		FreeLibrary(library);
		library = NULL;
		throw invalid_argument(stringprintf("Library %s doesn't identify as a scheduler (didn't export %s)", filename.c_str(), NAME(IdentifyAsScheduler)));
	}
	auto signatureValue = signature(TASKSIM_VERSION);
	if(signatureValue < MIN_REQUIRED_VERSION) {
		FreeLibrary(library);
		library = NULL;
		throw invalid_argument(stringprintf("Library %s scheduler version %ul is too old (min version required %ul)",
			filename.c_str(), signatureValue, MIN_REQUIRED_VERSION));
	}

	ojr = ProcAddress(OnJobRelease);
	ojf = ProcAddress(OnJobFinish);
	ojd = ProcAddress(OnJobDeadline);
	ojsf = ProcAddress(OnJobSliceFinish);
	ojrr = ProcAddress(OnJobResourceRequest);
	ojrf = ProcAddress(OnJobResourceFinish);
	ot = ProcAddress(OnTimer);
	oi = ProcAddress(OnIdle);
}

DLLScheduler::~DLLScheduler() {
	if(library != NULL)
		FreeLibrary(library);
}

void DLLScheduler::onJobRelease(double time, Job* job) {
	if(ojr)
		ojr(time, (void*)job);
}

void DLLScheduler::onJobFinish(double time, Job* job, unsigned int proc) {
	if(ojf)
		ojf(time, (void*)job, proc);
}

void DLLScheduler::onJobDeadline(double time, Job* job) {
	if(ojd)
		ojd(time, (void*)job);
}

void DLLScheduler::onJobSliceFinish(double time, Job* job, unsigned int proc) {
	if(ojsf)
		ojsf(time, (void*)job, proc);
}

void DLLScheduler::onResourceRequest(double time, Job* job, std::string resourceName, unsigned int proc) {
	if(ojrr)
		ojrr(time, (void*)job, resourceName.c_str(), proc);
}

void DLLScheduler::onResourceFinish(double time, Job* job, std::string resourceName, unsigned int proc) {
	if(ojrf)
		ojrf(time, (void*)job, resourceName.c_str(), proc);
}

void DLLScheduler::onTimer(double time, void* callbackPointer, void* timerPointer) {
	if(ot)
		ot(time, callbackPointer, timerPointer);
}

void DLLScheduler::onIdle(double time, unsigned int proc) {
	if(oi)
		oi(time, proc);
}
