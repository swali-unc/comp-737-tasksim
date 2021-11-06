#pragma once

#include <vector>

#include <SFML/Graphics.hpp>
#include "UIButton.hpp"

class SimView
{
public:
	SimView();
	~SimView();

	inline void registerButton(UIButton* btn) { buttons.push_back(btn); }
	void renderButtons(sf::RenderWindow& window, sf::Vector2f mouse) const;
private:
	std::vector<UIButton*> buttons;
};

