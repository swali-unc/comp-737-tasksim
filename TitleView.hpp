#pragma once

#include "ViewObject.hpp"
#include "ButtonView.hpp"
#include <vector>

#include <SFML/Graphics.hpp>
#include "UIButton.hpp"
#include "TextSprite.hpp"

class TitleView : public ButtonView
{
public:
	TitleView();
	virtual ~TitleView();

	virtual bool Render(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked);
private:
	UIButton* loadProbButton;
	TextSprite titleText;
};

