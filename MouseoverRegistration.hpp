#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <string>
#include <utility>

class MouseoverRegistration
{
public:
	MouseoverRegistration();
	~MouseoverRegistration();

	void registerRect(sf::FloatRect rect, std::string text);
	void registerCircle(sf::Vector2f center, float radius, std::string text);
	bool detectCollision(sf::Vector2f center, float radius, std::string& textDestination) const;

	void clearAll();

	static MouseoverRegistration* Instance();
private:
	static MouseoverRegistration* _instance;
	std::vector<std::pair<sf::FloatRect,std::string>> rects;
	std::vector<std::pair<std::pair<sf::Vector2f, float>, std::string>> circles;
};

