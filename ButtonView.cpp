#include "ButtonView.hpp"

using namespace sf;

bool ButtonView::Render(RenderWindow& window, Vector2f mouse, bool clicked) {
	RenderGetButtonIfClicked(window, mouse, clicked);
	return true;
}

UIButton* ButtonView::RenderGetButtonIfClicked(RenderWindow& window, Vector2f mouse, bool clicked) {
	UIButton* retButton = nullptr;

	for(auto& i : buttons) {
		if(i->isMouseover((unsigned int)mouse.x, (unsigned int)mouse.y)) {
			auto sprite = i->getMouseoverSprite();
			window.draw(*sprite);
			if(clicked) {
				if(!i->getClickState()) {
					i->doCallback();
					retButton = i;
				}
				i->setClickState(true);
			}
			else
				i->setClickState(false);
		}
		else {
			auto sprite = i->getRegularSprite();
			window.draw(*sprite);
		}
	}

	return retButton;
}