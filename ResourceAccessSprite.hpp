#pragma once

#include <SFML/Graphics.hpp>
#include "SpriteMaker.hpp"

class ResourceAccessSprite : public SpriteMaker
{
public:
	ResourceAccessSprite(float width, float height, sf::Color color);

	inline float getWidth() const { return width; }
	inline float getHeight() const { return height; }
	inline sf::Color getColor() const { return color; }
private:
	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	float width;
	float height;
	sf::Color color;
};

