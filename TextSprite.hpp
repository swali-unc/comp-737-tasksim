#pragma once

#include "SpriteMaker.hpp"
#include <string>

class TextSprite : public SpriteMaker
{
public:
	TextSprite(std::string text, sf::Color fontColor, unsigned int fontSize);

	void centerOrigin();
private:
	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	std::string text;
	sf::Color fontColor;
	unsigned int fontSize;
};

