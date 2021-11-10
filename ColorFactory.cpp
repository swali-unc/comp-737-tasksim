#include "ColorFactory.hpp"

using namespace sf;

static Color colorList[] = {
	Color(204,236,255), // light blue
	Color(255,204,204), // light red
	Color(204,255,204), // light green
	Color(255,204,150), // orange
	Color(200,191,230), // light purple
	Color(218,184,165), // light brown
	Color(204,204,204), // light gray
	Color(112,146,190), // darker blue
};

Color ColorFactory::getNextColor() {
	if (index >= sizeof(colorList) / sizeof(*colorList))
		index = 0;
	return colorList[index++];
}

ColorFactory::ColorFactory() noexcept {
	index = 0;
}