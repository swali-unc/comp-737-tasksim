#pragma once

#include <SFML/Graphics.hpp>

class ViewObject
{
public:
	virtual ~ViewObject() { }

	virtual bool Render(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked) = 0;
private:
};

