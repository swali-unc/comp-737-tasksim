#include "CommentSprite.hpp"

#include <stdexcept>
#include "FontFactory.hpp"

using std::string;
using std::runtime_error;
using namespace sf;

auto constexpr COMMENT_WIDTH = 16;
auto constexpr COMMENT_HEIGHT = 10;
auto constexpr COMMENT_FONT_SIZE = 8;
auto constexpr LABEL_FONT = "times.ttf";
auto constexpr COMMENT_BORDER_THICKNESS = 1;
#define COMMENT_FONT_COLOR Color::Black
#define COMMENT_BKG_COLOR Color::White
#define COMMENT_BORDER_COLOR Color::Black

RenderTexture* CommentSprite::createRenderTexture() {

	static Font* font = nullptr;
	if(!font && !FontFactory::loadFont(LABEL_FONT, &font))
		throw runtime_error("CommentSprite- Could not load font");

	RenderTexture* render = new RenderTexture();
	if(!render)
		throw runtime_error("CommentSprite- Could not create render texture");

	if(!render->create((unsigned int)COMMENT_WIDTH, (unsigned int)COMMENT_HEIGHT)) {
		delete render;
		throw std::runtime_error("CommentSprite- could not create render texture resolution");
	}

	RectangleShape box(Vector2f((float)COMMENT_WIDTH, (float)COMMENT_HEIGHT));
	box.setFillColor(COMMENT_BKG_COLOR);
	box.setOutlineColor(COMMENT_BORDER_COLOR);
	box.setOutlineThickness(-COMMENT_BORDER_THICKNESS);
	render->draw(box);

	// Create our text object with the associated font
	Text text;
	text.setFont(*font);
	text.setCharacterSize(COMMENT_FONT_SIZE);
	text.setFillColor(COMMENT_FONT_COLOR);
	text.setString(". . .");
	auto rect = text.getLocalBounds();
	text.setPosition((COMMENT_WIDTH / 2.f) - (rect.width/2), 0.f);
	render->draw(text);

	render->display();
	return render;
}

void CommentSprite::applySpriteTransforms(Sprite* sprite) {
	auto rect = sprite->getLocalBounds();
	sprite->setOrigin(rect.width / 2, 0.f);
}

CommentSprite::CommentSprite() : SpriteMaker() {
	initialize();
}

CommentSprite* CommentSprite::_instance = nullptr;
CommentSprite* CommentSprite::Instance() {
	if(!_instance)
		_instance = new CommentSprite();
	return _instance;
}