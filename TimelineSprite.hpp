#pragma once

#include <SFML/Graphics.hpp>
#include "SpriteMaker.hpp"

class TimelineSprite : public SpriteMaker {
public:
	TimelineSprite(double start, double interval, double end, float spacing, float dashlen) noexcept;

private:
	double start;
	double interval;
	double end;
	float spacing;
	float dashlen;

	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);
};