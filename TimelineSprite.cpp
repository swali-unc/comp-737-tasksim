#include "TimelineSprite.hpp"

#include <string>
#include <exception>
#include "FontFactory.hpp"

using namespace sf;

constexpr auto TIMELINE_FONT = "times.ttf";
constexpr auto HORIZONTAL_LINE_THICKNESS = 2.f;
constexpr auto VERTICAL_LINE_THICKNESS = 1.f;
constexpr auto FONT_SIZE = 12;
#define TIMELINE_COLOR Color::Black
#define TIMELINE_FONT_COLOR Color::Black

RenderTexture* TimelineSprite::createRenderTexture() {
	static Font* font = nullptr;
	if (!font && !FontFactory::loadFont(TIMELINE_FONT, &font))
		throw std::runtime_error("TimelineSprite- Could not load font");

	RenderTexture* renderTexture = new RenderTexture();
	if (!renderTexture)
		throw std::runtime_error("TimelineSprite- Could not allocate RenderTexture");

	// Our width needs to account for the fact that the font starts a little before our texture
	double width = (end - start) * spacing + FONT_SIZE;
	// We also need to give vertical area for the font
	double height = HORIZONTAL_LINE_THICKNESS + (double)dashlen + FONT_SIZE + 3;

	if (!renderTexture->create((unsigned int)width, (unsigned int)height)) {
		delete renderTexture;
		throw std::runtime_error("Could not create TimelineSprite rendertexture with given resolution");
	}

	// The giant horizontal line
	RectangleShape horizontal_line(Vector2f((float)width - FONT_SIZE + 1,(float)HORIZONTAL_LINE_THICKNESS));
	horizontal_line.setFillColor(TIMELINE_COLOR);
	renderTexture->draw(horizontal_line);

	// This is the vertical tick mark on the timeline
	RectangleShape vertical_line(Vector2f(VERTICAL_LINE_THICKNESS, dashlen));
	vertical_line.setFillColor(TIMELINE_COLOR);

	// The number that is shown next to each vertical tick
	Text number;
	number.setFont(*font);
	number.setCharacterSize(FONT_SIZE);
	number.setFillColor(TIMELINE_FONT_COLOR);

	for (double i = start; i <= end; i += interval) {
		vertical_line.setPosition((float)(i - start) * spacing, HORIZONTAL_LINE_THICKNESS);
		renderTexture->draw(vertical_line);

		number.setString(std::to_string((int)i));
		number.setPosition((float)(i - start) * spacing, HORIZONTAL_LINE_THICKNESS + dashlen + 1);
		if( i > start )
			number.setOrigin(Vector2f(number.getGlobalBounds().width / 2,0));
		renderTexture->draw(number);
	}

	renderTexture->display();
	return renderTexture;
}

void TimelineSprite::applySpriteTransforms(Sprite* sprite) {
	// The timeline is drawn from the bottom left corner
	sprite->setOrigin(Vector2f(0.f,sprite->getGlobalBounds().height));
}

TimelineSprite::TimelineSprite(double start, double interval, double end, float spacing, float dashlen) noexcept
	: SpriteMaker() {
	this->start = start;
	this->interval = interval;
	this->end = end;
	this->spacing = spacing;
	this->dashlen = dashlen;
	initialize();
}