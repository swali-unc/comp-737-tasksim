/* Task
 * 
 * A task is defined by the tuple (phase,period,cost,relative deadline).
 * There is an extra parameter called index that is simply useful for identifying
 * which task this is.
 */

#pragma once

#include <SFML/Graphics.hpp>

class Task
{
public:
	Task(double phase, double period, double cost, double relativeDeadline, int index) noexcept;
	Task(double period, double cost, double relativeDeadline, int index) noexcept;
	Task(double period, double cost, int index) noexcept;

	inline double getPhase() const { return phase; }
	inline double getPeriod() const { return period; }
	inline double getCost() const { return cost; }
	inline double getRelativeDeadline() const { return rdeadline; }
	inline int getIndex() const { return index; }
	inline sf::Color getColor() const { return color; }
	inline void setColor(sf::Color color) { this->color = color; }
private:
	double phase;
	double period;
	double cost;
	double rdeadline;
	int index;
	sf::Color color;
};