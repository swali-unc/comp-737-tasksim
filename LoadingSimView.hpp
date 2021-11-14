#pragma once
#include "ButtonView.hpp"

#include <signal.h>
#include <thread>
#include "ThreadLockedObject.hpp"

enum class SimProgress {
	UNKNOWN = 0,
	SIMULATING,
	SIMULATION_ERROR,
	SIMULATION_CANCEL,
	SIMULATION_SUCCESS,
	SIMULATION_SUCCESS_FINISH,
	SIMULATION_CANCEL_FINISH,
	SIMULATION_ERROR_FINISH,
};

class LoadingSimView : public ButtonView, public DigitalHaze::ThreadSpinLockedObject
{
public:
	LoadingSimView();
	virtual ~LoadingSimView();

	virtual bool Render(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked);

	friend void* SimulationThread(void* loadingView);
private:
	volatile sig_atomic_t simulationStatus;
	UIButton* cancelBtn;
	std::thread* simulationThread;
	double totalTime;

	void CancelSimBtnCallback();
};

