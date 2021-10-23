#pragma once

#include <SFML/Graphics.hpp>
#include "SpriteMaker.hpp"

class JobCompletionSprite : public SpriteMaker
{
public:
	static JobCompletionSprite* Instance();
	~JobCompletionSprite();
private:
	static JobCompletionSprite* _instance;

	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	JobCompletionSprite();
};