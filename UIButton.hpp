#pragma once
#include "SpriteMaker.hpp"

#include <string>
#include <functional>

class UIButton : private SpriteMaker
{
public:
	UIButton(std::string text, std::function<void()> callback, unsigned int width, unsigned int height);
	~UIButton();

	inline unsigned int getHeight() const { return height; }
	inline unsigned int getWidth() const { return width; }
	inline std::string getLabel() const { return text; }
	void doCallback() { callback(); }

	sf::Sprite* getSprite(unsigned int mouseX, unsigned int mouseY) const;

	void setButtonPosition(float x, float y);
	bool isMouseover(unsigned int x, unsigned int y) const;
private:
	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	std::function<void()> callback;
	std::string text;
	unsigned int width, height;
	sf::RenderTexture* mouseoverRenderTexture;
	sf::Texture mouseoverTexture;
	sf::Sprite *mouseoverSprite, *regularSprite;
	sf::Color borderColor;
};

