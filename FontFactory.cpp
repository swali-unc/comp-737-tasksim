#include "FontFactory.hpp"

#include <string>
#include <map>

using namespace sf;
using std::string;
using std::map;

map<string, Font*> FontFactory::fontMap;

bool FontFactory::loadFont(string filename, Font** output) {
	auto i = fontMap.find(filename);
	if (i != fontMap.end()) {
		*output = i->second;
		return true;
	}

	// font not found, need to load it
	Font* font = new Font();
	if (!font->loadFromFile(filename)) {
		delete font;
		*output = nullptr;
		return false;
	}

	fontMap[filename] = font;
	*output = font;
	return true;
}