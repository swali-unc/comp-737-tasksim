#include "ScheduleSprite.hpp"

#include <stdexcept>
#include <math.h>
#include "TimelineSprite.hpp"
#include "JobExecutionSprite.hpp"
#include "JobDeadlineSprite.hpp"
#include "JobReleaseSprite.hpp"
#include "JobCompletionSprite.hpp"
#include "FontFactory.hpp"
#include "MouseoverRegistration.hpp"
#include "Utility.hpp"

using namespace sf;
using std::make_pair;
using std::vector;
using std::string;
using std::pair;

auto constexpr JOB_HEIGHT = 40.f;
auto constexpr CONTEXT_SWITCH_HEIGHT = 20.f;
auto constexpr TIMELINE_HEIGHT = 10.f;
auto constexpr TIMELINE_SPACING = 25.f;
auto constexpr MAX_JOB_HEIGHT = JOB_HEIGHT > CONTEXT_SWITCH_HEIGHT ? JOB_HEIGHT : CONTEXT_SWITCH_HEIGHT;
auto constexpr EXCESS_HEIGHT = 3.f;
auto constexpr RENDER_LABEL = false;
auto constexpr LABEL_FONT = "times.ttf";
auto constexpr LABEL_FONT_SIZE = 12;
auto constexpr LEFT_SHIFT = 4.f;
#define LABEL_FONT_COLOR Color::Black

RenderTexture* ScheduleSprite::createRenderTexture() {
	// Create the timeline, definitely need this
	TimelineSprite timeline(start, interval, end, TIMELINE_SPACING, TIMELINE_HEIGHT);
	Sprite* timelineSprite = timeline.createSprite();

	// Note that our timeline is our width, and we need to add height to it
	double width = ceil((double)timelineSprite->getGlobalBounds().width) + LEFT_SHIFT;
	double height = ceil((double)timelineSprite->getGlobalBounds().height + MAX_JOB_HEIGHT) + EXCESS_HEIGHT;

	// Do we have deadlines and/or releases?
	Sprite* deadlineSprite = JobDeadlineSprite::Instance()->createSprite();
	Sprite* releaseSprite = JobReleaseSprite::Instance()->createSprite();
	Sprite* completionSprite = JobCompletionSprite::Instance()->createSprite();
	double arrowHeight = 0;
	auto updateMaxHeight = [&arrowHeight](unsigned int count, double height) { if (count && arrowHeight < height) arrowHeight = height; };
	updateMaxHeight(numDeadlines, deadlineSprite->getGlobalBounds().height);
	updateMaxHeight(numReleases, releaseSprite->getGlobalBounds().height);
	updateMaxHeight(numCompletions, completionSprite->getGlobalBounds().height);
	height += arrowHeight;

	// protected variable
	RenderTexture* render = new RenderTexture();
	if (!render) {
		delete timelineSprite;
		delete deadlineSprite;
		delete releaseSprite;
		throw std::runtime_error("ScheduleSprite- could not create render texture");
	}

	if (!render->create((unsigned int)width, (unsigned int)height)) {
		delete render;
		delete timelineSprite;
		throw std::runtime_error("ScheduleSprite- could not create dimensions on render texture");
	}

	// Draw the timeline at the bottom
	float timelineHeight = timelineSprite->getGlobalBounds().height;
	timelineSprite->setPosition(LEFT_SHIFT, (float)height - timelineHeight);
	render->draw(*timelineSprite);
	// Done with that
	delete timelineSprite;

	// Now we need to render all of the job slices and place them on the timeline
	double widthPerCost = TIMELINE_SPACING / interval;
	for (auto i = 0u; i < numJobs; ++i) {
		JobExecutionSprite jExecSprite(JOB_HEIGHT, runningJobs[i], widthPerCost);
		Sprite* jSprite = jExecSprite.createSprite();
		jSprite->setPosition((float)((runningJobs[i].getStart() - start) * widthPerCost) + LEFT_SHIFT, (float)(height - timelineHeight));
		render->draw(*jSprite);
		moReg.push_back(make_pair(jSprite->getGlobalBounds(),
			"J" + runningJobs[i].getJob()->createLabel() + " executes for " + to_string_trim(runningJobs[i].getDuration())
			+ " [" + to_string_trim(runningJobs[i].getStart()) + "," + to_string_trim(runningJobs[i].getStart() + runningJobs[i].getDuration()) + ")"
		));
		delete jSprite;
	}

	// Now render all releases and deadlines
	auto renderArrows = [this,widthPerCost,height,timelineHeight,&render](Sprite* sprite, unsigned int num, pair<double, string>* entries, string prependText = "") {
		for (auto i = 0u; i < num; ++i) {
			auto startTime = entries[i].first;
			auto label = entries[i].second;
			sprite->setPosition((float)((startTime - start) * widthPerCost) + LEFT_SHIFT, (float)(height - timelineHeight));
			render->draw(*sprite);

			if (RENDER_LABEL) {
				static Font* font = nullptr;
				if (!font && !FontFactory::loadFont(LABEL_FONT, &font))
					throw std::runtime_error("TimelineSprite- Could not load font");

				Text text;
				text.setFont(*font);
				text.setCharacterSize(LABEL_FONT_SIZE);
				text.setFillColor(LABEL_FONT_COLOR);
				text.setString(label);
				FloatRect textRect = text.getGlobalBounds();
				text.setOrigin(textRect.left + textRect.width / 2, textRect.top + textRect.height / 2);
				text.setPosition((float)((startTime - start) * widthPerCost) + LEFT_SHIFT, (float)(height - timelineHeight - sprite->getGlobalBounds().height - LABEL_FONT_SIZE - 1.f));
				render->draw(text);
			}

			FloatRect outputRect = sprite->getGlobalBounds();
			Vector2f outputPos = sprite->getPosition();
			Vector2f outputOrigin = sprite->getOrigin();
			outputRect.top = outputPos.y - outputOrigin.y;
			outputRect.left = outputPos.x - outputOrigin.x;
			moReg.push_back(make_pair(outputRect, prependText + label + " (" + to_string_trim(startTime) + ")"));
		}
	};

	if (numReleases)
		renderArrows(releaseSprite, numReleases, releaseLabels, "Release: ");
	if (numDeadlines)
		renderArrows(deadlineSprite, numDeadlines, deadlineLabels, "Deadline: ");
	if (numCompletions)
		renderArrows(completionSprite, numCompletions, completionLabels, "Complete: ");

	render->display();
	return render;
}

void ScheduleSprite::doMouseoverRegistrations(float x, float y) const {
	auto mReg = MouseoverRegistration::Instance();

	for (auto& i : moReg) {
		auto rect = i.first;
		auto label = i.second;

		rect.left += x;
		rect.top += y;

		mReg->registerRect(rect, label);
	}
}

void ScheduleSprite::applySpriteTransforms(Sprite*) {
}

ScheduleSprite::ScheduleSprite(double start, double end, double interval,
	JobExecution* runningJobs, unsigned int numJobs,
	std::pair<double, std::string>* releaseLabels, unsigned int numReleases,
	std::pair<double, std::string>* deadlineLabels, unsigned int numDeadlines,
	std::pair<double, std::string>* completionLabels, unsigned int numCompletions)
	: SpriteMaker() {
	this->start = start;
	this->end = end;
	this->interval = interval;
	this->runningJobs = runningJobs;
	this->numJobs = numJobs;
	this->releaseLabels = releaseLabels;
	this->numReleases = numReleases;
	this->deadlineLabels = deadlineLabels;
	this->numDeadlines = numDeadlines;
	this->completionLabels = completionLabels;
	this->numCompletions = numCompletions;
	initialize();
}

ScheduleSprite::~ScheduleSprite() {
}