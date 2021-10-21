#pragma once

#include "SpriteMaker.hpp"

#include <utility>
#include <string>
#include <vector>
#include "JobExecution.hpp"

class ScheduleSprite : public SpriteMaker
{
public:
	ScheduleSprite(double start, double end, double interval,
		JobExecution* runningJobs, unsigned int numJobs,
		std::pair<double,std::string>* releaseLabels, unsigned int numReleases,
		std::pair<double,std::string>* deadlineLabels, unsigned int numDeadlines);
	~ScheduleSprite();

	inline double getStart() const { return start; }
	inline double getEnd() const { return end; }
	inline double getInterval() const { return interval; }
	inline JobExecution* getRunningJobs() const { return runningJobs; }
	inline unsigned int getNumJobs() const { return numJobs; }

	void doMouseoverRegistrations(float x, float y) const;
private:
	virtual sf::RenderTexture* createRenderTexture();
	virtual void applySpriteTransforms(sf::Sprite* sprite);

	double start;
	double end;
	double interval;
	JobExecution* runningJobs;
	unsigned int numJobs;
	std::pair<double, std::string>* releaseLabels;
	unsigned int numReleases;
	std::pair<double, std::string>* deadlineLabels;
	unsigned int numDeadlines;
	std::vector<std::pair<sf::FloatRect, std::string>> moReg;
};

