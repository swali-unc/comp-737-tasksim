#include "MouseoverRegistration.hpp"

#include <cmath>
#include <map>

using namespace sf;
using std::string;
using std::make_pair;
using std::pair;
using std::vector;
using std::map;

inline bool detectCollisionSimple(const Vector2f& point, const pair<Vector2f, float>& circle);
inline bool detectCollisionSimple(const Vector2f& point, const FloatRect& rect);
template<typename T>
inline T getVectorLength(Vector2<T> v);

bool MouseoverRegistration::detectCollision(Vector2f center, float radius, string& textDestination) const {
	float diagonalOffset = radius * (float)sqrt(2) / 2.f;
	textDestination = "";
	vector<Vector2f> pointList = {
		Vector2f(center - Vector2f(radius,0)), // Left center
		Vector2f(center + Vector2f(radius,0)), // Right center
		Vector2f(center + Vector2f(0,radius)), // Bottom center
		Vector2f(center - Vector2f(0,radius)), // Top center
		Vector2f(center + Vector2f(diagonalOffset,diagonalOffset)), // Bottom right
		Vector2f(center - Vector2f(diagonalOffset,diagonalOffset)), // Top left
		Vector2f(center + Vector2f(-diagonalOffset,diagonalOffset)), // bottom left
		Vector2f(center + Vector2f(diagonalOffset,-diagonalOffset)), // bottom right
	};

	map<void*, bool> visitedMap;

	for (auto& i : pointList) {
		for(auto& m : circles) {
			for(auto& v : m.second) {
				if(visitedMap.find((void*)&v) == visitedMap.end() && detectCollisionSimple(i, v.first)) {
					textDestination += v.second;
					visitedMap[(void*)&v] = true;
				}
			}
		}
		for(auto& m : rects) {
			for(auto& v : m.second) {
				if(visitedMap.find((void*)&v) == visitedMap.end() && detectCollisionSimple(i, v.first)) {
					textDestination += v.second;
					visitedMap[(void*)&v] = true;
				}
			}
		}
	}

	return textDestination.length() != 0;
}

void MouseoverRegistration::registerRect(ViewObject* parent, FloatRect rect, string text) {
	rects[parent].push_back(make_pair(rect, text + "\n"));
}

void MouseoverRegistration::registerCircle(ViewObject* parent, Vector2f center, float radius, string text) {
	circles[parent].push_back(make_pair(make_pair(center, radius),text + "\n"));
}

void MouseoverRegistration::clearAll() {
	for(auto& i : circles)
		i.second.clear();
	for(auto& i : rects)
		i.second.clear();
	circles.clear();
	rects.clear();
}

void MouseoverRegistration::clearView(ViewObject* parent) {
	for(auto& i : circles[parent])
		i.second.clear();
	for(auto& i : rects[parent])
		i.second.clear();
}

MouseoverRegistration::MouseoverRegistration() {
}

MouseoverRegistration::~MouseoverRegistration() {
	clearAll();
}

MouseoverRegistration* MouseoverRegistration::_instance = nullptr;
MouseoverRegistration* MouseoverRegistration::Instance() {
	if (!_instance)
		_instance = new MouseoverRegistration();
	return _instance;
}

template<typename T>
inline T getVectorLength(Vector2<T> v) {
	return std::sqrt(v.x * v.x + v.y * v.y);
}

inline bool detectCollisionSimple(const Vector2f& point, const pair<Vector2f, float>& circle) {
	float radius = circle.second;
	Vector2f center = circle.first;

	return radius <= getVectorLength(center - point);
}

inline bool detectCollisionSimple(const Vector2f& point, const FloatRect& rect) {
	return point.x <= rect.left + rect.width
		&& point.x >= rect.left
		&& point.y <= rect.top + rect.height
		&& point.y >= rect.top;
}