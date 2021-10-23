#pragma once

#include <SFML/Graphics.hpp>

class ColorFactory
{
public:
	ColorFactory() noexcept;

	sf::Color getNextColor();
private:
	int index;
};

