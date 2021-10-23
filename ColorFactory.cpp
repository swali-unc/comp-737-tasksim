#include "ColorFactory.hpp"

using namespace sf;

static Color colorList[] = {
	Color(204,236,255),
	Color(255,204,204),
	Color(204,255,204),
};

Color ColorFactory::getNextColor() {
	if (index >= sizeof(colorList) / sizeof(*colorList))
		index = 0;
	return colorList[index++];
}

ColorFactory::ColorFactory() noexcept {
	index = 0;
}