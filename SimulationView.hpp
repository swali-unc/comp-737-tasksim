#pragma once
#include "ViewObject.hpp"
#include "ButtonView.hpp"

#include "ScheduleSprite.hpp"
#include "TextSprite.hpp"

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
	TextSprite* currentTimeSprite;

	void createTimeSprite();
};

