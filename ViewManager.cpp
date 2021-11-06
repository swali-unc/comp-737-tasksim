#include "ViewManager.hpp"

using namespace sf;

void ViewManager::performQueuedActions() {
	if(clearQueued) {
		clearViews();
		clearQueued = false;
	}

	addViewsFromQueue();
}

void ViewManager::addViewsFromQueue() {
	for(auto& i : newViewQueue)
		addView(i);
	newViewQueue.clear();
}

void ViewManager::clearViews() {
	for(auto& i : views)
		delete i;
	views.clear();
}

void ViewManager::renderAll(RenderWindow& window, Vector2f mouse, bool clicked) {
	for(auto& i : views) {
		if(!i->Render(window, mouse, clicked))
			break; // If requested to stop looping
	}
}

ViewManager::ViewManager() {
	clearQueued = false;
}

ViewManager::~ViewManager() {
	addViewsFromQueue();
	clearViews();
}

ViewManager* ViewManager::_instance = nullptr;
ViewManager* ViewManager::Instance() {
	if(!_instance)
		_instance = new ViewManager();
	return _instance;
}