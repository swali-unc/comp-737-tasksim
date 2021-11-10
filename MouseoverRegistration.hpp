#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <string>
#include <utility>
#include <map>

#include "ViewObject.hpp"

class MouseoverRegistration
{
public:
	MouseoverRegistration();
	~MouseoverRegistration();

	void registerRect(ViewObject* parent, sf::FloatRect rect, std::string text);
	void registerCircle(ViewObject* parent, sf::Vector2f center, float radius, std::string text);
	bool detectCollision(sf::Vector2f center, float radius, std::string& textDestination) const;

	void clearAll();
	void clearView(ViewObject* parent);

	static MouseoverRegistration* Instance();
private:
	static MouseoverRegistration* _instance;
	std::map<ViewObject*,std::vector<std::pair<sf::FloatRect,std::string>>> rects;
	std::map<ViewObject*,std::vector<std::pair<std::pair<sf::Vector2f, float>, std::string>>> circles;
};

