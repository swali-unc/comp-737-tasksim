#include "SpriteMaker.hpp"

#include <exception>

using namespace sf;

Sprite* SpriteMaker::createSprite() {
	if (!renderTexture)
		initialize();

	Sprite* returnSprite = new Sprite(texture);
	applySpriteTransforms(returnSprite);
	return returnSprite;
}

void SpriteMaker::initialize() {
	if (!renderTexture)
		createTexture();
}

void SpriteMaker::createTexture() {
	if (!renderTexture) {
		renderTexture = createRenderTexture();
		if (!renderTexture)
			throw std::runtime_error("Could not create render texture in SpriteMaker");
	}

	texture = renderTexture->getTexture();
}

SpriteMaker::SpriteMaker() noexcept {
	renderTexture = nullptr;
}

SpriteMaker::~SpriteMaker() {
	if (renderTexture) delete renderTexture;
}