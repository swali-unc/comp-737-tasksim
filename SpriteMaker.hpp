#pragma once

#include <SFML/Graphics.hpp>

class SpriteMaker
{
public:
	void initialize();
	sf::Sprite* createSprite();

	SpriteMaker() noexcept;
	~SpriteMaker();
private:
	sf::RenderTexture* renderTexture;
	sf::Texture texture;

	virtual sf::RenderTexture* createRenderTexture() = 0;
	virtual void applySpriteTransforms(sf::Sprite* sprite) = 0;
	void createTexture();
};

