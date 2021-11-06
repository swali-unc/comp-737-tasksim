#pragma once

#include <vector>
#include "UIButton.hpp"
#include "ViewObject.hpp"

class ButtonView : public ViewObject
{
public:
	inline void registerButton(UIButton* btn) { buttons.push_back(btn); }

	virtual bool Render(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked);
	UIButton* RenderGetButtonIfClicked(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked);
	inline void clearButtons() { buttons.clear(); }
protected:
	std::vector<UIButton*> buttons;
};

