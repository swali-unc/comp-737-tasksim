#include "ScheduleSprite.hpp"

#include <stdexcept>
#include <math.h>
#include "TimelineSprite.hpp"
#include "JobExecutionSprite.hpp"
#include "JobDeadlineSprite.hpp"
#include "JobReleaseSprite.hpp"
#include "JobCompletionSprite.hpp"
#include "CommentSprite.hpp"
#include "ResourceAccessSprite.hpp"
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
	Sprite* deadlineSprite = JobDeadlineSprite::Instance()->getCachedSprite();
	Sprite* releaseSprite = JobReleaseSprite::Instance()->getCachedSprite();
	Sprite* completionSprite = JobCompletionSprite::Instance()->getCachedSprite();
	double arrowHeight = 0;
	auto updateMaxHeight = [&arrowHeight](unsigned int count, double height) { if (count && arrowHeight < height) arrowHeight = height; };
	updateMaxHeight(numDeadlines, deadlineSprite->getGlobalBounds().height);
	updateMaxHeight(numReleases, releaseSprite->getGlobalBounds().height);
	updateMaxHeight(numCompletions, completionSprite->getGlobalBounds().height);
	height += arrowHeight;

	// Create our render texture
	RenderTexture* render = new RenderTexture();
	if (!render) {
		delete timelineSprite;
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
		if(runningJobs[i].getStart() > end || runningJobs[i].getStart() < start)
			continue;
		JobExecutionSprite jExecSprite(JOB_HEIGHT, runningJobs[i], widthPerCost);
		Sprite* jSprite = jExecSprite.createSprite();
		jSprite->setPosition((float)((runningJobs[i].getStart() - start) * widthPerCost) + LEFT_SHIFT, (float)(height - timelineHeight));
		render->draw(*jSprite);
		moReg.push_back(make_pair(jSprite->getGlobalBounds(),
			runningJobs[i].getJob()->createLabel() + " executes for " + to_string_trim(runningJobs[i].getDuration())
			+ " [" + to_string_trim(runningJobs[i].getStart()) + "," + to_string_trim(runningJobs[i].getStart() + runningJobs[i].getDuration()) + ")"
		));

		// Are there any resource accesses in this slice?
		auto resources = runningJobs[i].getResourceParameters();
		for (auto& r : resources) {
			double offsetIntoSlice = r.start;
			double duration = r.duration;
			if(duration <= 0)
				continue;
			FloatRect spriteRect = jSprite->getGlobalBounds();
			spriteRect.left += (float)(offsetIntoSlice * widthPerCost);
			spriteRect.width = (float)(duration * widthPerCost);

			ResourceAccessSprite ras(spriteRect.width, spriteRect.height / 3, runningJobs[i].getJob()->getColor());
			Sprite* rSprite = ras.createSprite();
			rSprite->setPosition(spriteRect.left, spriteRect.top + spriteRect.height);
			render->draw(*rSprite);

			moReg.push_back(
				make_pair(spriteRect,
					stringprintf("Resource: %s-%s [%s,%s)",
						r.resourceName.c_str(), runningJobs[i].getJob()->createLabel().c_str(),
						to_string_trim(runningJobs[i].getStart() + offsetIntoSlice).c_str(),
						to_string_trim(runningJobs[i].getStart() + offsetIntoSlice + duration).c_str()
					)
			));

			delete rSprite;
		}

		// Cleanup
		delete jSprite;
	}

	// Now render all releases and deadlines
	auto renderArrows = [this,widthPerCost,height,timelineHeight,&render](Sprite* sprite, unsigned int num, pair<double, string>* entries, string prependText = "") {
		for (auto i = 0u; i < num; ++i) {
			auto startTime = entries[i].first;
			if(startTime > end || startTime < start)
				continue;
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
	if (numComments)
		renderArrows(CommentSprite::Instance()->getCachedSprite(), numComments, comments, "Comment: ");

	render->display();
	return render;
}

void ScheduleSprite::doMouseoverRegistrations(float x, float y) const {
	auto mReg = MouseoverRegistration::Instance();

	for (auto& i : moReg) {
		auto rect = i.first;

		rect.left += x;
		rect.top += y;

		mReg->registerRect(parentView, rect, i.second);
	}
}

void ScheduleSprite::applySpriteTransforms(Sprite*) {
}

ScheduleSprite::ScheduleSprite(double start, double end, double interval,
	ViewObject* parentView,
	JobExecution* runningJobs, unsigned int numJobs,
	pair<double, string>* releaseLabels, unsigned int numReleases,
	pair<double, string>* deadlineLabels, unsigned int numDeadlines,
	pair<double, string>* completionLabels, unsigned int numCompletions,
	pair<double, string>* comments, unsigned int numComments
)
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
	this->comments = comments;
	this->numComments = numComments;
	this->parentView = parentView;
	initialize();
}

ScheduleSprite::~ScheduleSprite() {
	//Can't clear our entire view's mouseovers
	//MouseoverRegistration::Instance()->clearView(parentView);
}