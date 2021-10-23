#include "ResourceAccessSprite.hpp"

#include <stdexcept>
#include <cmath>

using namespace sf;
using std::runtime_error;

auto constexpr colorIntensityModifier = 0.6;
auto constexpr lineSpacing = 3.f;
auto constexpr lineThickness = 1.f;

Color modifyIntensity(Color inColor,double multiplier);

RenderTexture* ResourceAccessSprite::createRenderTexture() {
	RenderTexture* render = new RenderTexture();
	if (!render)
		throw runtime_error("ResourceAccessSprite- Could not allocate render texture");

	if (!render->create((unsigned int)width, (unsigned int)height))
		throw runtime_error("ResourceAccessSprite- Could not create render texture resolution");

	float lineLength = height * sqrt(2.f);
	for (float x = -height; x < width; x += lineSpacing) {
		RectangleShape line(Vector2f(lineLength,lineThickness));
		line.setFillColor(color);
		line.rotate(45);
		line.setPosition(x, 0);
		render->draw(line);
	}

	render->display();
	return render;
}

void ResourceAccessSprite::applySpriteTransforms(Sprite* sprite) {
	// Set origin to bottom left
	sprite->setOrigin(0, height);
}

ResourceAccessSprite::ResourceAccessSprite(float width, float height, Color color) {
	this->width = width;
	this->height = height;
	this->color = modifyIntensity(color, colorIntensityModifier);
	initialize();
}

Color modifyIntensity(Color p, double multiplier) {
	double y = 0.299 * p.r + 0.587 * p.g + 0.114 * p.b;
	double u = -0.147 * p.r - 0.289 * p.g + 0.436 * p.b;
	double v = 0.615 * p.r - 0.515 * p.g - 0.1 * p.b;

	y *= multiplier;
	auto bindvalue = [](double v, double min = 0, double max = 255) { return (v < min ? min : (v > max ? max : v)); };
	return Color(
		(Uint8)bindvalue(y + 1.14 * v),
		(Uint8)bindvalue(y - 0.395 * u),
		(Uint8)bindvalue(u + 2.032 * u),
		p.a);
}