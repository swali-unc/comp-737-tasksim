#pragma once

#include <map>
#include <string>

#include <SFML/Graphics.hpp>

class FontFactory
{
private:
	static std::map<std::string, sf::Font*> fontMap;
public:
	static bool loadFont(std::string filename, sf::Font** output);
};

