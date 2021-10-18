#pragma comment(lib,"sfml-graphics.lib")
#pragma comment(lib,"sfml-window.lib")
#pragma comment(lib,"sfml-system.lib")
#pragma comment(lib,"sfml-main.lib")

#include <SFML/Graphics.hpp>

using namespace sf;

int main() {
	RenderWindow window(VideoMode(1280, 720), "COMP 737 Task Scheduling Simulator");
	const Color clearColor = { 255, 255, 204 };
	window.setFramerateLimit(100);

	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed)
				window.close();
		}

		window.clear(clearColor);
		// Draw here

		window.display();
	}

	return 0;
}