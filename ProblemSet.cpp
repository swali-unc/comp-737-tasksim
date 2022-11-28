#include "ProblemSet.hpp"

#include <stdexcept>
#include "ColorFactory.hpp"

using std::string;
using std::runtime_error;
using namespace tinyxml2;

int getNumSiblingElements(tinyxml2::XMLElement* parent, const char* elementName);

ProblemSet::ProblemSet(string filename) {
	loadProblem(filename);
}

void ProblemSet::loadProblem(string filename) {
	XMLDocument doc;
	if (doc.LoadFile(filename.c_str()) != XMLError::XML_SUCCESS)
		throw runtime_error("ProblemSet- Could not parse xml file");

	// Problem variables
	auto problem = doc.FirstChildElement("Problem");
	if (!problem)
		throw runtime_error("ProblemSet- missing Problem block");

	// Task Set
	auto tasks = doc.FirstChildElement("Tasks");
	if (!tasks)
		throw runtime_error("ProblemSet- missing Tasks block");

	// Aperiodics
	auto jobs = doc.FirstChildElement("Jobs");
	if (!jobs)
		throw runtime_error("ProblemSet- missing Jobs block");

	initializeProblem(problem);
	initializeTaskSet(tasks);
	initializeAperiodics(jobs);
}

ProblemSet::~ProblemSet() {
	if (taskSet) {
		for (auto i = 0u; i < numTasks; ++i)
			delete taskSet[i];
		delete[] taskSet;
	}

	if (aperiodicJobs) {
		for (auto i = 0u; i < numAperiodics; ++i)
			delete aperiodicJobs[i];
		delete[] aperiodicJobs;
	}
}

void ProblemSet::initializeProblem(XMLElement* problem) {
	auto procCount = problem->FirstChildElement("NumProcessors");
	processorCount = procCount ? procCount->UnsignedText() : 1;

	auto schdLen = problem->FirstChildElement("ScheduleLength");
	if (!schdLen)
		throw runtime_error("Missing schedule length");
	scheduleLength = schdLen->DoubleText();

	auto timelineInt = problem->FirstChildElement("TimelineInterval");
	timelineInterval = timelineInt ? timelineInt->DoubleText() : 1;
}

void ProblemSet::initializeTaskSet(XMLElement* tasks) {
	// We want colors for our tasks
	ColorFactory colors;
	
	// Count the tasks
	unsigned int task_count = getNumSiblingElements(tasks, "Task");
	this->numTasks = task_count;

	// No tasks? that was easy
	if (!task_count) {
		taskSet = nullptr;
		return;
	}

	// We have a task set
	taskSet = new Task * [task_count];
	tinyxml2::XMLElement** elements;
	{
		unsigned int task_index = 0;
		elements = new tinyxml2::XMLElement * [task_count];
		for(auto child = tasks->FirstChildElement("Task"); child; child = child->NextSiblingElement("Task"))
			elements[task_index++] = child;
	}

#pragma omp for nowait
	for(int i = 0; i < task_count; ++i) {
		tinyxml2::XMLElement* child = elements[(unsigned int)i];

		double taskPhase, taskPeriod, taskCost, taskRelativeDeadline;

		// Default release is 0
		auto release = child->FirstChildElement("Phase");
		taskPhase = !release ? 0 : release->DoubleText();

		// No such thing as a default period
		auto period = child->FirstChildElement("Period");
		if(!period) {
			delete[] elements;
			throw runtime_error("ProblemSet- problem missing period");
		}
		taskPeriod = period->DoubleText();

		// No such thing as a default cost
		auto cost = child->FirstChildElement("Cost");
		if(!cost) {
			delete[] elements;
			throw runtime_error("ProblemSet- problem missing cost");
		}
		taskCost = cost->DoubleText();

		// Default relative deadline is period
		auto rDeadline = child->FirstChildElement("RelativeDeadline");
		taskRelativeDeadline = !rDeadline ? taskPeriod : rDeadline->DoubleText();

		// Create the task
		taskSet[(unsigned int)i] = new Task(taskPhase, taskPeriod, taskCost, taskRelativeDeadline, i);

		// Do we have any resources?
		auto resources = child->FirstChildElement("Resources");
		if (resources) {
			int num_resources = getNumSiblingElements(resources, "Resource");
			tinyxml2::XMLElement** resourceElements = new tinyxml2::XMLElement * [num_resources];
			{
				unsigned int resource_index = 0;
				for(auto resource = resources->FirstChildElement("Resource"); resource; resource = resource->NextSiblingElement("Resource"))
					resourceElements[resource_index++] = resource;
			}

			// Unfortunately, MSVS will not allow for to be nested.
			// #pragma omp for
			for( int j = 0; j < num_resources; ++j) {
				// Resource name
				tinyxml2::XMLElement* resource = resourceElements[(unsigned int)j];
				auto name = resource->FirstChildElement("Name");
				if(!name) {
					delete[] elements;
					delete[] resourceElements;
					throw runtime_error("ProblemSet- resource missing name");
				}
				// Default start offset is 0
				auto resourceStart = resource->FirstChildElement("Start");
				double startOffset = !resourceStart ? 0 : resourceStart->DoubleText();
				// Cost
				auto resourceCost = resource->FirstChildElement("Cost");
				if(!resourceCost) {
					delete[] elements;
					delete[] resourceElements;
					throw runtime_error("ProblemSet- resource missing cost");
				}

				taskSet[(unsigned int)i]->addResourceAccess(startOffset, resourceCost->DoubleText(), name->GetText());
			}
		}

		// Set the color
		taskSet[(unsigned int)i]->setColor(colors.getNextColor());
	}

	delete[] elements;
}

void ProblemSet::initializeAperiodics(XMLElement* jobs) {
	// Count the jobs
	unsigned int job_count = 0;
	for (auto child = jobs->FirstChildElement("Job"); child; child = child->NextSiblingElement("Job"))
		++job_count;
	this->numAperiodics = job_count;

	// No tasks? that was easy
	if (!job_count) {
		aperiodicJobs = nullptr;
		return;
	}

	// We have a task set
	aperiodicJobs = new Job * [job_count];
	unsigned int job_index = 0;
	for (auto child = jobs->FirstChildElement("Job"); child; child = child->NextSiblingElement("Job")) {
		double jobRelease, jobDeadline, jobAbsDeadline, jobCost;

		// Default release is 0
		auto release = child->FirstChildElement("Release");
		jobRelease = !release ? 0 : release->DoubleText();

		// No such thing as a default cost
		auto cost = child->FirstChildElement("Cost");
		if (!cost) throw runtime_error("ProblemSet- aperiodic job missing cost");
		jobCost = cost->DoubleText();

		// If we don't have a relative deadline, then abs deadline will give it.
		auto rDeadline = child->FirstChildElement("RelativeDeadline");
		jobDeadline = !rDeadline ? -1 : rDeadline->DoubleText();

		// If we don't have an absolute deadline, then relative deadline will give it
		auto aDeadline = child->FirstChildElement("AbsoluteDeadline");
		jobAbsDeadline = !aDeadline ? -1 : aDeadline->DoubleText();

		// Were both missing?
		if (jobAbsDeadline == -1 && jobDeadline == -1)
			throw runtime_error("ProblemSet- Aperiodic job is missing both relative and absolute deadline.");
		// If relative deadline missing, fix it, otherwise fix absolute deadline
		if (jobDeadline == -1) jobDeadline = jobAbsDeadline - jobRelease;
		else jobAbsDeadline = jobRelease + jobDeadline;

		// Create the task
		aperiodicJobs[job_index] = new Job(jobRelease, jobAbsDeadline, jobDeadline, jobCost, job_index);

		// Do we have any resources?
		auto resources = child->FirstChildElement("Resources");
		if(resources) {
			for(auto resource = resources->FirstChildElement("Resource"); resource; resource = resource->NextSiblingElement("Resource")) {
				// Resource name
				auto name = resource->FirstChildElement("Name");
				if(!name) throw runtime_error("ProblemSet- resource missing name");
				// Default start offset is 0
				auto resourceStart = resource->FirstChildElement("Start");
				double startOffset = !resourceStart ? 0 : resourceStart->DoubleText();
				// Cost
				auto resourceCost = resource->FirstChildElement("Cost");
				if(!resourceCost) throw runtime_error("ProblemSet- resource missing cost");

				aperiodicJobs[job_index]->addResourceAccess(startOffset, resourceCost->DoubleText(), name->GetText());
			}
		}

		++job_index;
	}
}

int getNumSiblingElements(tinyxml2::XMLElement* parent, const char* elementName) {
	int count = 0;
	for(auto child = parent->FirstChildElement(elementName); child; child = child->NextSiblingElement(elementName))
		++count;
	return count;
}