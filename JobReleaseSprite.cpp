#include "JobReleaseSprite.hpp"

#include <exception>

using namespace sf;
using std::runtime_error;

constexpr auto WIDTH = 8;
constexpr auto LINE_WIDTH = 2.f;
constexpr auto HEIGHT = 50;
constexpr auto LINE_HEIGHT = HEIGHT - 2.f;
constexpr auto ARROW_WIDTH = WIDTH;
constexpr auto ARROW_HEIGHT = WIDTH;
#define FILL_COLOR Color::Green

RenderTexture* JobReleaseSprite::createRenderTexture() {
	RenderTexture* render = new RenderTexture();
	if (!render)
		throw runtime_error("JobReleaseSprite: Could not create render texture");

	if (!render->create(WIDTH, HEIGHT)) {
		delete render;
		throw runtime_error("JobReleaseSprite: Could not create render texture with specified resolution");
	}

	// the line of the arrow
	RectangleShape line(Vector2f(LINE_WIDTH, LINE_HEIGHT));
	line.setFillColor(FILL_COLOR);
	// We draw this upwards, so set our origin to the bottom of the line
	line.setOrigin(LINE_WIDTH / 2, (float)LINE_HEIGHT);
	// draw from the very bottom of our render texture
	line.setPosition(WIDTH / 2.f, (float)HEIGHT);
	render->draw(line);

	ConvexShape arrowhead;
	arrowhead.setPointCount(3);
	// triangle bottom left
	arrowhead.setPoint(0, Vector2f(0, (float)ARROW_HEIGHT));
	// triangle bottom right
	arrowhead.setPoint(1, Vector2f((float)ARROW_WIDTH, (float)ARROW_HEIGHT));
	// triangle top middle
	arrowhead.setPoint(2, Vector2f((float)ARROW_WIDTH / 2, 0));
	arrowhead.setFillColor(FILL_COLOR);
	render->draw(arrowhead);

	render->display();
	return render;
}

void JobReleaseSprite::applySpriteTransforms(Sprite* sprite) {
	sprite->setOrigin(Vector2f(WIDTH/2.f,(float)HEIGHT));
}

JobReleaseSprite::JobReleaseSprite() : SpriteMaker() {
	initialize();
}

JobReleaseSprite::~JobReleaseSprite() {
}

JobReleaseSprite* JobReleaseSprite::_instance = nullptr;
JobReleaseSprite* JobReleaseSprite::Instance() {
	if (!_instance)
		_instance = new JobReleaseSprite();
	return _instance;
}