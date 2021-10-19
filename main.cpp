#pragma comment(lib,"sfml-graphics.lib")
#pragma comment(lib,"sfml-window.lib")
#pragma comment(lib,"sfml-system.lib")
#pragma comment(lib,"sfml-main.lib")

#include <SFML/Graphics.hpp>
#include "JobReleaseSprite.hpp"
#include "JobDeadlineSprite.hpp"
#include "TimelineSprite.hpp"

using namespace sf;

int main() {
	RenderWindow window(VideoMode(1280, 720), "COMP 737 Task Scheduling Simulator");
	const Color clearColor = { 255, 255, 204 };
	window.setFramerateLimit(100);

	Sprite* jobRelease = JobReleaseSprite::Instance()->createSprite();
	jobRelease->setPosition(Vector2f(100.f, 100.f));
	Sprite* jobDeadline = JobDeadlineSprite::Instance()->createSprite();
	jobDeadline->setPosition(Vector2f(200.f, 100.f));
	TimelineSprite* timeline = new TimelineSprite(0, 1, 40, 25, 10);
	Sprite* timelineSprite = timeline->createSprite();
	timelineSprite->setPosition(100.f, 300.f);

	Event e;
	while (window.isOpen()) {
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed)
				window.close();
		}

		window.clear(clearColor);
		// Draw here
		window.draw(*jobRelease);
		window.draw(*jobDeadline);
		window.draw(*timelineSprite);

		// End frame
		window.display();
	}

	delete jobRelease;
	delete jobDeadline;
	delete timelineSprite;
	delete timeline;

	return 0;
}