#pragma once

#include "ViewObject.hpp"
#include "TextSprite.hpp"
#include "ThreadLockedObject.hpp"

#include <stdlib.h>
#include <signal.h>

#include <thread>
#include <string>

enum OpenFileViewStatus {
	UNKNOWN = 0,
	IN_DIALOG,
	NO_FILE_PICKED,
	FILE_PICKED,
};

class OpenFileView : public ViewObject, public DigitalHaze::ThreadSpinLockedObject
{
public:
	OpenFileView();
	OpenFileView(std::string text, const char* filetypes);
	virtual ~OpenFileView();

	virtual bool Render(sf::RenderWindow& window, sf::Vector2f mouse, bool clicked);

	friend void* OpenFileThread(void* data);
private:
	TextSprite* waitText;
	char filepath[_MAX_PATH];
	const char* filetypes;

	volatile sig_atomic_t fileOpenStatus;
	std::thread* fileOpenThread;
};

