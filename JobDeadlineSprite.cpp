#include "JobDeadlineSprite.hpp"

#include <exception>

using namespace sf;
using std::runtime_error;

#define WIDTH 8
#define LINE_WIDTH 2.f
#define HEIGHT 50
#define LINE_HEIGHT HEIGHT-2.f
#define ARROW_WIDTH WIDTH
#define ARROW_HEIGHT WIDTH
#define FILL_COLOR Color::Red

RenderTexture* JobDeadlineSprite::createRenderTexture() {
	RenderTexture* render = new RenderTexture();
	if (!render)
		throw runtime_error("JobDeadlineSprite: Could not create render texture");

	// Create our render texture
	if (!render->create(WIDTH, HEIGHT)) {
		delete render;
		throw runtime_error("JobDeadlineSprite: Could not create render texture with specified resolution");
	}

	// The line of the arrow
	RectangleShape line(Vector2f(LINE_WIDTH, LINE_HEIGHT));
	line.setFillColor(FILL_COLOR);
	// This line should be drawn upwards, our origin in the bottom of the line
	line.setOrigin(LINE_WIDTH / 2, (float)LINE_HEIGHT);
	// And we start drawing from above the arrow
	line.setPosition(WIDTH / 2.f, (float)HEIGHT - ARROW_HEIGHT);
	render->draw(line);

	ConvexShape arrowhead;
	arrowhead.setPointCount(3);
	// top left of triangle
	arrowhead.setPoint(0, Vector2f(0, 0));
	// top right of triangle
	arrowhead.setPoint(1, Vector2f((float)ARROW_WIDTH, 0));
	// bottom middle of triangle
	arrowhead.setPoint(2, Vector2f((float)ARROW_WIDTH / 2, (float)ARROW_HEIGHT));
	arrowhead.setFillColor(FILL_COLOR);
	arrowhead.setPosition(0, HEIGHT - ARROW_HEIGHT);
	render->draw(arrowhead);

	render->display();
	return render;
}

void JobDeadlineSprite::applySpriteTransforms(Sprite* sprite) {
	sprite->setOrigin(Vector2f(WIDTH / 2.f, (float)HEIGHT));
}

JobDeadlineSprite::JobDeadlineSprite() : SpriteMaker() {
	initialize();
}

JobDeadlineSprite::~JobDeadlineSprite() {
}

JobDeadlineSprite* JobDeadlineSprite::_instance = nullptr;
JobDeadlineSprite* JobDeadlineSprite::Instance() {
	if (!_instance)
		_instance = new JobDeadlineSprite();
	return _instance;
}