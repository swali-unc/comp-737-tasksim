#include "UIButton.hpp"
#include "FontFactory.hpp"

using std::string;
using std::function;
using namespace sf;

#define BORDER_COLOR Color::Black
#define BORDER_COLOR_MOUSEOVER Color::Red
auto constexpr BORDER_WIDTH = 1.f;
auto constexpr LABEL_FONT = "times.ttf";
auto constexpr LABEL_FONT_SIZE = 12;
#define LABEL_FONT_COLOR Color::Black
#define BUTTON_COLOR Color::White

void UIButton::setButtonPosition(float x, float y) {
	mouseoverSprite->setPosition(x, y);
	regularSprite->setPosition(x, y);
}

Sprite* UIButton::getSprite(unsigned int x, unsigned int y) const {
	return isMouseover(x,y)	? mouseoverSprite : regularSprite;
}

bool UIButton::isMouseover(unsigned int x, unsigned int y) const {
	auto box = regularSprite->getGlobalBounds();
	return x >= box.left && x <= box.left + box.width
		&& y >= box.top && y <= box.top + box.height;
}

RenderTexture* UIButton::createRenderTexture() {
	static Font* font = nullptr;
	if(!font && !FontFactory::loadFont(LABEL_FONT, &font))
		throw std::runtime_error("UIButton- Could not load font");

	RenderTexture* renderTexture = new RenderTexture();
	if(!renderTexture)
		throw std::runtime_error("UIButton- could not allocate render texture");

	if(!renderTexture->create((unsigned int)width, (unsigned int)height)) {
		delete renderTexture;
		throw std::runtime_error("UIButton- could not create render texture resolution");
	}

	RectangleShape rect(Vector2f((float)width, (float)height));
	if(BORDER_WIDTH > 0) {
		rect.setOutlineColor(borderColor);
		rect.setOutlineThickness(-BORDER_WIDTH); // negative means draw border inwards
	}

	// Finish drawing rectangle
	rect.setFillColor(BUTTON_COLOR);
	renderTexture->draw(rect);

	// This has a label potentially (if we can fit it)
	Text text;
	text.setFont(*font);
	text.setCharacterSize(LABEL_FONT_SIZE);
	text.setFillColor(LABEL_FONT_COLOR);
	text.setString(this->text);

	// Check if we can fit it
	FloatRect textRect = text.getGlobalBounds();
	if(textRect.width <= width) {
		text.setOrigin(textRect.left + textRect.width / 2.f,
			textRect.top + textRect.height / 2.f);
		text.setPosition((float)(width / 2), (float)(height / 2));
		renderTexture->draw(text);
	}

	renderTexture->display();
	return renderTexture;
}

void UIButton::applySpriteTransforms(sf::Sprite* sprite) {
}

UIButton::UIButton(string text, function<void()> callback, unsigned int width, unsigned int height)
	: SpriteMaker() {
	this->text = text;
	this->callback = callback;
	this->width = width;
	this->height = height;
	this->borderColor = BORDER_COLOR;
	initialize();
	this->regularSprite = createSprite();

	this->borderColor = BORDER_COLOR_MOUSEOVER;
	this->mouseoverRenderTexture = createRenderTexture();
	this->mouseoverTexture = this->mouseoverRenderTexture->getTexture();
	this->mouseoverSprite = new Sprite(this->mouseoverTexture);
	applySpriteTransforms(this->mouseoverSprite);

	this->clickState = false;
}

UIButton::~UIButton() {
	delete this->mouseoverSprite;
	delete this->regularSprite;
	delete this->mouseoverRenderTexture;
}