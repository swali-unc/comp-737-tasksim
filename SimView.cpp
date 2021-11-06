#include "SimView.hpp"

using namespace sf;
using std::vector;

void SimView::renderButtons(RenderWindow& window, Vector2f mouse) const {
	for(auto& i : buttons) {
		auto sprite = i->getSprite(mouse.x, mouse.y);
		window.draw(*sprite);
	}
}

SimView::SimView() {
}

SimView::~SimView() {
}