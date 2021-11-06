#pragma once

#include "ViewObject.hpp"
#include <vector>

#include <SFML/Graphics.hpp>

class ViewManager
{
public:
	~ViewManager();

	inline void addView(ViewObject* view) { views.push_back(view); }
	inline void queueClear() { clearQueued = true; }
	inline void queueView(ViewObject* view) { newViewQueue.push_back(view); }

	void renderAll(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked);
	void performQueuedActions();

	static ViewManager* Instance();
private:
	ViewManager();

	void clearViews();
	void addViewsFromQueue();

	static ViewManager* _instance;
	std::vector<ViewObject*> views;
	std::vector<ViewObject*> newViewQueue;
	bool clearQueued;
};

