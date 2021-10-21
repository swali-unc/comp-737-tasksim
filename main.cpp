#pragma comment(lib,"sfml-graphics.lib")
#pragma comment(lib,"sfml-window.lib")
#pragma comment(lib,"sfml-system.lib")
#pragma comment(lib,"sfml-main.lib")

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <math.h>
#include <stdexcept>

#include "JobReleaseSprite.hpp"
#include "JobDeadlineSprite.hpp"
#include "TimelineSprite.hpp"
#include "ProblemSet.hpp"
#include "ScheduleSprite.hpp"
#include "MouseoverRegistration.hpp"
#include "FontFactory.hpp"

using namespace sf;
using std::string;
using std::runtime_error;
using std::exception;
using std::make_pair;
using std::pair;
using std::tie;

auto constexpr MOUSE_WIDTH = 21.f;
auto constexpr MOUSE_ADJUST = (MOUSE_WIDTH-1.f) / 2.f;
auto constexpr MOUSEOVER_FONT = "times.ttf";
auto constexpr MOUSEOVER_FONT_SIZE = 12;
auto constexpr MOUSEOVER_INBORDER_SIZE = 10.f;
#define MOUSEOVER_FONT_COLOR Color::Black
#define MOUSEOVER_FILL_COLOR Color::White

pair<Sprite*,RenderTexture*> UpdateMouseoverText(Vector2f mouse, float radius);

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

	ProblemSet problem("Problems\\TestProblem1.xml");

	int numJobs = 3;
	//JobExecution* runningJobs = new JobExecution[numJobs];
	Job jobs[] = { Job(0,10,10,1,0), Job(0,11,11,2,1), Job(0,15,15,3,2) };
	JobExecution runningJobs[] = {
		JobExecution(jobs[0],0,1),
		JobExecution(jobs[1],1,2),
		JobExecution(jobs[2],3,2)
	};
	pair<double, string> releases[] = {
		make_pair(0,"J0"), make_pair(0,"J1"), make_pair(0,"J2")
	};
	pair<double, string> deadlines[] = {
		make_pair(jobs[0].getAbsoluteDeadline(),"J0"),
		make_pair(jobs[1].getAbsoluteDeadline(),"J1"),
		make_pair(jobs[2].getAbsoluteDeadline(),"J2")
	};
	ScheduleSprite* schedule = new ScheduleSprite(0, 40, 1, runningJobs, numJobs, releases, 3, deadlines, 3);
	Sprite* scheduleSprite = schedule->createSprite();
	scheduleSprite->setPosition(100.f, 400.f);
	schedule->doMouseoverRegistrations(100.f, 400.f);

	Event e;
	Vector2f mouse(0, 0);
	CircleShape mouseRect(MOUSE_ADJUST);
	mouseRect.setOrigin(Vector2f(MOUSE_ADJUST,MOUSE_ADJUST));
	mouseRect.setFillColor(Color(255, 255, 255, 128));

	Sprite* mouseoverSprite = nullptr;
	RenderTexture* mouseoverRender = nullptr;

	while (window.isOpen()) {
		while (window.pollEvent(e)) {\
			switch (e.type) {
			case Event::Closed:
				window.close();
				break;
			case Event::MouseMoved:
				mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));
				mouseRect.setPosition(mouse.x, mouse.y);
				if (mouseoverSprite) delete mouseoverSprite;
				if (mouseoverRender) delete mouseoverRender;

				try {
					tie(mouseoverSprite,mouseoverRender) = UpdateMouseoverText(mouse, MOUSE_ADJUST);
					if (mouseoverSprite)
						mouseoverSprite->setPosition(mouse);
				}
				catch (exception e) {
					mouseoverSprite = nullptr;
					mouseoverRender = nullptr;
					fprintf(stderr, "%s\n", e.what());
				}
				catch (...) {
					fprintf(stderr, "Unhandled exception\n");
				}
				break;
			}
		}

		window.clear(clearColor);
		// Draw here
		window.draw(*jobRelease);
		window.draw(*jobDeadline);
		window.draw(*timelineSprite);
		window.draw(*scheduleSprite);
		window.draw(mouseRect);

		// Final draw
		if (mouseoverSprite)
			window.draw(*mouseoverSprite);

		// End frame
		window.display();
	}

	delete jobRelease;
	delete jobDeadline;
	delete timelineSprite;
	delete timeline;

	return 0;
}

pair<Sprite*,RenderTexture*> UpdateMouseoverText(Vector2f mouse, float radius) {
	string textDestination;
	
	// Is there any mouseover text?
	if (!MouseoverRegistration::Instance()->detectCollision(mouse, radius, textDestination))
		return make_pair<Sprite*,RenderTexture*>(nullptr,nullptr);

	// Load our font
	static Font* font = nullptr;
	if (!font && !FontFactory::loadFont(MOUSEOVER_FONT, &font))
		throw runtime_error("Could not load font in mouseover display");

	// Create the output text
	Text text;
	text.setFont(*font);
	text.setCharacterSize(MOUSEOVER_FONT_SIZE);
	text.setFillColor(MOUSEOVER_FONT_COLOR);
	text.setString(textDestination);
	text.setOrigin(0, 0);
	text.setPosition(MOUSEOVER_INBORDER_SIZE, MOUSEOVER_INBORDER_SIZE);

	// This is the background on which the text is output
	FloatRect textRect = text.getGlobalBounds();
	RectangleShape background(Vector2f(textRect.width + 2 * MOUSEOVER_INBORDER_SIZE, textRect.height + 2 * MOUSEOVER_INBORDER_SIZE));
	background.setFillColor(MOUSEOVER_FILL_COLOR);

	// This is what everything is drawn on
	RenderTexture* render = new RenderTexture();
	if (!render) throw runtime_error("Could not allocate render texture in mouseover text");
	if (!render->create((unsigned int)background.getGlobalBounds().width, (unsigned int)background.getGlobalBounds().height)) {
		delete render;
		throw runtime_error("Could not create render texture resolution");
	}

	// Perform the draws
	render->draw(background);
	render->draw(text);
	
	// Display, grab the sprite, and cleanup
	render->display();
	Sprite* sprite = new Sprite(render->getTexture());
	if (!sprite) throw runtime_error("Could not allocate sprite for mouseover text");

	return make_pair(sprite,render);
}