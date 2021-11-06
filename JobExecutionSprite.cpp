#include "JobExecutionSprite.hpp"

#include <stdexcept>
#include "FontFactory.hpp"
#include "Utility.hpp"

using namespace sf;

#define BORDER_COLOR Color::Black
auto constexpr BORDER_WIDTH = 1.f;
auto constexpr LABEL_FONT = "times.ttf";
auto constexpr LABEL_FONT_SIZE = 12;
#define LABEL_FONT_COLOR Color::Black

RenderTexture* JobExecutionSprite::createRenderTexture() {
	static Font* font = nullptr;
	if (!font && !FontFactory::loadFont(LABEL_FONT, &font))
		throw std::runtime_error("JobExecutionSprite- Could not load font");

	RenderTexture* renderTexture = new RenderTexture();
	if (!renderTexture)
		throw std::runtime_error("JobExecutionSprite- could not allocate render texture");

	if (!renderTexture->create((unsigned int)width, (unsigned int)height)) {
		delete renderTexture;
		throw std::runtime_error(stringprintf("JobExecutionSprite- could not create render texture resolution %u x %u",
			(unsigned int)width, (unsigned int)height));
	}

	RectangleShape rect(Vector2f((float)width, (float)height));
	if (BORDER_WIDTH > 0) {
		rect.setOutlineColor(BORDER_COLOR);
		rect.setOutlineThickness(-BORDER_WIDTH); // negative means draw border inwards
	}

	// Finish drawing rectangle
	rect.setFillColor(color);
	renderTexture->draw(rect);

	// This has a label potentially (if we can fit it)
	Text text;
	text.setFont(*font);
	text.setCharacterSize(LABEL_FONT_SIZE);
	text.setFillColor(LABEL_FONT_COLOR);
	text.setString(label);

	// Check if we can fit it
	FloatRect textRect = text.getGlobalBounds();
	if (textRect.width <= width) {
		text.setOrigin(textRect.left + textRect.width / 2.f,
			textRect.top + textRect.height / 2.f);
		text.setPosition((float)(width / 2), (float)(height / 2));
		renderTexture->draw(text);
	}

	renderTexture->display();
	return renderTexture;
}

void JobExecutionSprite::applySpriteTransforms(Sprite* sprite) {
	// Drawn from the bottom left
	sprite->setOrigin(0.f, (float)height);
}

JobExecutionSprite::JobExecutionSprite(double height, JobExecution& job, double widthPerCost)
	: JobExecutionSprite(height, job.getDuration() * widthPerCost, job.getJob()->createLabel(), job.getJob()->getColor()) {
}

JobExecutionSprite::JobExecutionSprite(double height, double width, std::string label, sf::Color color)
	: SpriteMaker() {
	this->height = height;
	this->width = width;
	this->label = label;
	this->color = color;
	initialize();
}