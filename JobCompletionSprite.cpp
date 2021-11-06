#include "JobCompletionSprite.hpp"

#include <stdexcept>

using namespace sf;
using std::runtime_error;

auto constexpr WIDTH = 8;
auto constexpr HEIGHT = 70;
auto constexpr LINE_WIDTH = 3;
auto constexpr LINE_TOP_THICKNESS = 4;
#define LINE_COLOR Color::Black

RenderTexture* JobCompletionSprite::createRenderTexture() {
	RenderTexture* render = new RenderTexture();
	if (!render)
		throw runtime_error("JobCompletionSprite: Could not create render texture");

	// Create our render texture
	if (!render->create(WIDTH, HEIGHT)) {
		delete render;
		throw runtime_error("JobCompletionSprite: Could not create render texture with specified resolution");
	}

	RectangleShape line(Vector2f((float)LINE_WIDTH, (float)HEIGHT));
	line.setFillColor(LINE_COLOR);
	line.setOrigin((float)LINE_WIDTH / 2, (float)HEIGHT);
	line.setPosition((float)WIDTH / 2, (float)HEIGHT);
	render->draw(line);

	RectangleShape topLine(Vector2f((float)WIDTH, (float)LINE_TOP_THICKNESS));
	topLine.setFillColor(LINE_COLOR);
	render->draw(topLine);

	render->display();
	return render;
}

void JobCompletionSprite::applySpriteTransforms(Sprite* sprite) {
	sprite->setOrigin(Vector2f(WIDTH / 2.f, (float)HEIGHT));
}

JobCompletionSprite::JobCompletionSprite() : SpriteMaker() {
	initialize();
}

JobCompletionSprite::~JobCompletionSprite() {
}

JobCompletionSprite* JobCompletionSprite::_instance = nullptr;
JobCompletionSprite* JobCompletionSprite::Instance() {
	if (!_instance)
		_instance = new JobCompletionSprite();
	return _instance;
}