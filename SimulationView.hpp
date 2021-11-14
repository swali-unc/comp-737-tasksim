#pragma once
#include "ViewObject.hpp"
#include "ButtonView.hpp"

#include "ScheduleSprite.hpp"
#include "TextSprite.hpp"

#include <vector>
#include <utility>

class SimulationView : public ButtonView
{
public:
	SimulationView();
	virtual ~SimulationView();

	virtual bool Render(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked);
private:
	bool simulationInProgress;
	unsigned int procCount;
	double timeStart;
	double timeEnd;

	void createTimeline(unsigned int proc);
	void timeForward();
	void timeBackward();
	void CreateRenders();

	ScheduleSprite** timelines;
	UIButton* timeForwardBtn;
	UIButton* timeBackwardBtn;
	UIButton* newProblemBtn;
	UIButton* newSchedulerBtn;
	TextSprite* currentTimeSprite;
	TextSprite** processorNames;
	std::vector<std::pair<double,UIButton*>> errors;

	void createTimeSprite();

	void ErrorButtonCallback(double time);
	void NewProblemButton();
	void NewSchedulerButton();
};

