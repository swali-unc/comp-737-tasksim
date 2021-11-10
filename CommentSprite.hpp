#pragma once

#include "SpriteMaker.hpp"

class CommentSprite : public SpriteMaker
{
public:
	static CommentSprite* Instance();

	virtual ~CommentSprite() { }
private:
	static CommentSprite* _instance;

	// Inherited via SpriteMaker
	virtual sf::RenderTexture* createRenderTexture() override;
	virtual void applySpriteTransforms(sf::Sprite* sprite) override;

	CommentSprite();
};

