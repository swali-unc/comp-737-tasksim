#pragma comment(lib,"sfml-graphics.lib")
#pragma comment(lib,"sfml-window.lib")
#pragma comment(lib,"sfml-system.lib")
#pragma comment(lib,"sfml-main.lib")

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <math.h>
#include <stdexcept>
#include <utility>

#include "JobReleaseSprite.hpp"
#include "JobDeadlineSprite.hpp"
#include "JobCompletionSprite.hpp"
#include "TimelineSprite.hpp"
#include "CommentSprite.hpp"
#include "ProblemSet.hpp"
#include "ScheduleSprite.hpp"
#include "MouseoverRegistration.hpp"
#include "FontFactory.hpp"
#include "SimulationState.hpp"
#include "ViewObject.hpp"
#include "TitleView.hpp"
#include "ViewManager.hpp"

using namespace sf;
using std::string;
using std::runtime_error;
using std::exception;
using std::make_pair;
using std::pair;
using std::tie;
using std::vector;

auto constexpr MOUSE_WIDTH = 21.f;
auto constexpr MOUSE_ADJUST = (MOUSE_WIDTH-1.f) / 2.f;
auto constexpr MOUSEOVER_FONT = "times.ttf";
auto constexpr MOUSEOVER_FONT_SIZE = 12;
auto constexpr MOUSEOVER_INBORDER_SIZE = 10.f;
auto constexpr MOUSEOVER_OUTLINE_THICKNESS = -2.f;
#define MOUSEOVER_FONT_COLOR Color::Black
#define MOUSEOVER_FILL_COLOR Color::White
#define MOUSEOVER_OUTLINE_COLOR Color(127,0,0)
#define MOUSEOVER_OFFSET Vector2f(5.f,5.f)

pair<Sprite*,RenderTexture*> UpdateMouseoverText(Vector2f mouse, float radius);

int main(int argc, char* argv[]) {
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "COMP 737 Task Scheduling Simulator");
	const Color clearColor = { 255, 255, 204 };
	window.setFramerateLimit(100);

	// Mouseover data
	Vector2f mouse(0, 0);
	CircleShape mouseRect(MOUSE_ADJUST);
	mouseRect.setOrigin(Vector2f(MOUSE_ADJUST, MOUSE_ADJUST));
	mouseRect.setFillColor(Color(255, 255, 255, 128));
	Sprite* mouseoverSprite = nullptr;
	RenderTexture* mouseoverRender = nullptr;

	// Our current views
	auto viewManager = ViewManager::Instance();
	viewManager->addView(new TitleView());

	Event e;
	bool mouseClicked = false;
	while(window.isOpen()) {
		while(window.pollEvent(e)) {
			switch(e.type) {
			case Event::Closed:
				window.close();
				break;
			case Event::MouseMoved:
				mouse = window.mapPixelToCoords(Mouse::getPosition(window));
				mouseRect.setPosition(mouse.x, mouse.y);
				if(mouseoverSprite) delete mouseoverSprite;
				if(mouseoverRender) delete mouseoverRender;

				try {
					tie(mouseoverSprite, mouseoverRender) = UpdateMouseoverText(mouse, MOUSE_ADJUST);
					if(mouseoverSprite)
						mouseoverSprite->setPosition(mouse + MOUSEOVER_OFFSET);
				}
				catch(exception e) {
					mouseoverSprite = nullptr;
					mouseoverRender = nullptr;
					fprintf(stderr, "Exception: %s\n", e.what());
				}
				catch(...) {
					fprintf(stderr, "Unhandled exception\n");
				}
				break;
			}
		}

		// Click state on mouse
		mouseClicked = Mouse::isButtonPressed(Mouse::Button::Left);

		window.clear(clearColor);
		// Draw here
		viewManager->renderAll(window, mouse, mouseClicked);

		// This tiny light circle around the mouse
		window.draw(mouseRect);

		// Final draw
		if(mouseoverSprite)
			window.draw(*mouseoverSprite);

		// End frame
		window.display();

		viewManager->performQueuedActions();
	}

	// Delete sprites
	if(mouseoverSprite) delete mouseoverSprite;
	if(mouseoverRender) delete mouseoverRender;

	// Delete global instances
	// Sprites
	delete JobReleaseSprite::Instance();
	delete JobDeadlineSprite::Instance();
	delete JobCompletionSprite::Instance();
	delete CommentSprite::Instance();

	// States
	delete MouseoverRegistration::Instance();
	delete SimulationState::Instance();

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
	background.setOutlineThickness(MOUSEOVER_OUTLINE_THICKNESS);
	background.setOutlineColor(MOUSEOVER_OUTLINE_COLOR);

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

/*int main() {
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
	Sprite* jobComplete = JobCompletionSprite::Instance()->createSprite();
	jobComplete->setPosition(300.f, 100.f);

	ProblemSet* problem;
	try {
		problem = new ProblemSet("Problems\\test.xml");
	}
	catch (exception e) {
		fprintf(stderr, "Could not load problem: %s", e.what());
		return 1;
	}
	SimulationState::Instance()->setProblem(problem);
	Task** tasks = problem->getTaskSet();

	// Jobs release- just gonna assume all tasks phase is 0 for testing
	Job jobs[] = { Job(*tasks[0],0), Job(*tasks[1],0), Job(*tasks[2],0) };
	// Scheduler makes this decision- for testing, schedule all jobs sequentially
	JobExecution runningJobs[] = { jobs[0].executeJob(1), jobs[1].executeJob(2,1), jobs[2].executeJob(2,3) };

	// Release data
	pair<double, string> releases[] = {
		make_pair(jobs[0].getReleaseTime(),jobs[0].createLabel()),
		make_pair(jobs[1].getReleaseTime(),jobs[1].createLabel()),
		make_pair(jobs[2].getReleaseTime(),jobs[2].createLabel())
	};
	// Deadline data
	pair<double, string> deadlines[] = {
		make_pair(jobs[0].getAbsoluteDeadline(),jobs[0].createLabel()),
		make_pair(jobs[1].getAbsoluteDeadline(),jobs[1].createLabel()),
		make_pair(jobs[2].getAbsoluteDeadline(),jobs[2].createLabel())
	};
	// Job completion data
	pair<double, string> completions[] = {
		make_pair(1,jobs[0].createLabel()), make_pair(3,jobs[1].createLabel()), make_pair(5,jobs[2].createLabel())
	};
	ScheduleSprite* schedule = new ScheduleSprite(0, problem->getScheduleLength(), problem->getTimelineInterval(), runningJobs, 3, releases, 3, deadlines, 3, completions, 3);
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
		while (window.pollEvent(e)) {
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
						mouseoverSprite->setPosition(mouse + MOUSEOVER_OFFSET);
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
		window.draw(*jobComplete);
		window.draw(*timelineSprite);
		window.draw(*scheduleSprite);

		// This tiny light circle around the mouse
		window.draw(mouseRect);

		// Final draw
		if (mouseoverSprite)
			window.draw(*mouseoverSprite);

		// End frame
		window.display();
	}

	delete jobRelease;
	delete jobDeadline;
	delete jobComplete;
	delete timelineSprite;
	delete scheduleSprite;

	if (mouseoverSprite) delete mouseoverSprite;
	if (mouseoverRender) delete mouseoverRender;

	delete schedule;
	delete timeline;

	// Delete global instances
	delete JobReleaseSprite::Instance();
	delete JobDeadlineSprite::Instance();
	delete JobCompletionSprite::Instance();
	delete MouseoverRegistration::Instance();
	delete SimulationState::Instance();

	return 0;
}*/