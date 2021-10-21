#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "SpriteMaker.hpp"
#include "JobExecution.hpp"

class JobExecutionSprite : public SpriteMaker
{
public:
	JobExecutionSprite(double height, JobExecution& job, double widthPerCost);
	JobExecutionSprite(double height, double width, std::string label, sf::Color color);

	inline double getHeight() const { return height; }
	inline double getWidth() const { return width; }
	inline std::string getLabel() const { return label; }
	inline sf::Color getColor() const { return color; }
private:
	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	double height;
	double width;
	std::string label;
	sf::Color color;
};

