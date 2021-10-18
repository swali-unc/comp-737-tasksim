/* JobReleaseSprite
 *
 * This will create a sprite that we can use which is the upward pointing arrow that indicates a job released.
 */
#pragma once

#include <SFML/Graphics.hpp>
#include "SpriteMaker.hpp"

class JobDeadlineSprite : public SpriteMaker
{
public:
	static JobDeadlineSprite* Instance();
private:
	static JobDeadlineSprite* _instance;

	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	JobDeadlineSprite();
	~JobDeadlineSprite();
};

