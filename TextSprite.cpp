#include "TextSprite.hpp"
#include "FontFactory.hpp"

using std::string;
using namespace sf;

auto constexpr LABEL_FONT = "times.ttf";

sf::RenderTexture* TextSprite::createRenderTexture() {
	// Load our font
	static Font* font = nullptr;
	if(!font && !FontFactory::loadFont(LABEL_FONT, &font))
		throw std::runtime_error("TextSprite- Could not load font");

	RenderTexture* renderTexture = new RenderTexture();
	if(!renderTexture)
		throw std::runtime_error("TextSprite- could not allocate render texture");

	// Create our text object with the associated font
	Text text;
	text.setFont(*font);
	text.setCharacterSize(fontSize);
	text.setFillColor(fontColor);
	text.setString(this->text);

	// This is the size we need for our font
	FloatRect textRect = text.getGlobalBounds();

	if(!renderTexture->create((unsigned int)textRect.width, (unsigned int)textRect.height * 2)) {
		delete renderTexture;
		throw std::runtime_error("TextSprite- could not create render texture resolution");
	}

	renderTexture->draw(text);
	renderTexture->display();
	return renderTexture;
}

void TextSprite::applySpriteTransforms(sf::Sprite* sprite) {
}

void TextSprite::centerOrigin() {
	auto sprite = getCachedSprite();
	auto box = sprite->getGlobalBounds();
	sprite->setOrigin(box.width / 2, box.height / 2);
}

TextSprite::TextSprite(string text,Color color,unsigned int size) : SpriteMaker() {
	this->text = text;
	this->fontColor = color;
	this->fontSize = size;
}