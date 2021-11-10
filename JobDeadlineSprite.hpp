/* JobDeadlineSprite
 *
 * This will create a sprite that we can use which is the downward pointing arrow that indicates a job released.
 */
#pragma once

#include <SFML/Graphics.hpp>
#include "SpriteMaker.hpp"

class JobDeadlineSprite : public SpriteMaker
{
public:
	static JobDeadlineSprite* Instance();
	virtual ~JobDeadlineSprite();
private:
	static JobDeadlineSprite* _instance;

	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	JobDeadlineSprite();
};

