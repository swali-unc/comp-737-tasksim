/* SpriteMaker
 * 
 * Abstract class that creates a sprite from a render texture. Once the sprite
 * has been created, there is no need to keep the original object that made it.
 * Creating additional sprites creates quick distinct duplicates that can be used
 * in other different objects.
 */
#pragma once

#include <SFML/Graphics.hpp>

class SpriteMaker
{
public:
	void initialize();
	sf::Sprite* createSprite();
	inline sf::Sprite* getCachedSprite() { if(!cachedSprite) { cachedSprite = createSprite(); } return cachedSprite; }

	SpriteMaker() noexcept;
	virtual ~SpriteMaker();
protected:
	sf::RenderTexture* renderTexture;
	sf::Texture texture;
	sf::Sprite* cachedSprite;
private:
	virtual sf::RenderTexture* createRenderTexture() = 0;
	virtual void applySpriteTransforms(sf::Sprite* sprite) = 0;
	void createTexture();
};

