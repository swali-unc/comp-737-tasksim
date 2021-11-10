/* TimelineSprite
 * 
 * Creates a sprite for the timeline, which is just tick marks and numbers really.
 */
#pragma once

#include <SFML/Graphics.hpp>
#include "SpriteMaker.hpp"

class TimelineSprite : public SpriteMaker {
public:
	TimelineSprite(double start, double interval, double end, float spacing, float dashlen) noexcept;
	virtual ~TimelineSprite() { }

	inline double getStart() const { return start; }
	inline double getInterval() const { return interval; }
	inline double getEnd() const { return end; }
	inline float getSpacing() const { return spacing; }
	inline float getDashLen() const { return dashlen; }
private:
	double start;
	double interval;
	double end;
	float spacing;
	float dashlen;

	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);
};