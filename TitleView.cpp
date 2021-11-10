#include "TitleView.hpp"

#include <cmath>
#include "ViewManager.hpp"
#include "SimulationState.hpp"
#include "OpenFileView.hpp"

using namespace sf;
using std::vector;

auto constexpr TITLE = "Real-Time System Task Simulator";

void OpenFileCallback();

bool TitleView::Render(RenderWindow& window, Vector2f mouse, bool clicked) {
	auto clickedButton = ButtonView::RenderGetButtonIfClicked(window, mouse, clicked);
	window.draw(*titleText.getCachedSprite());

	if(clickedButton == loadProbButton) {
		return false; // Do not process any more views
	}

	return true;
}

TitleView::TitleView() :
	ButtonView(), titleText(TITLE,Color::Black,24)
{
	loadProbButton = new UIButton("Load Problem", OpenFileCallback, 100, 50);
	registerButton(loadProbButton);
	loadProbButton->setButtonPosition(50, 50);

	titleText.centerOrigin();
	titleText.getCachedSprite()->setPosition((float)round(WINDOW_WIDTH / 2), (float)round(WINDOW_HEIGHT * .5f / 2));
}

TitleView::~TitleView() {
	delete loadProbButton;
}

void OpenFileCallback() {
	ViewManager::Instance()->queueClear();
	ViewManager::Instance()->queueView(new OpenFileView("Waiting for problem XML to be selected..", "XML\0*.XML\0All\0*.*\0"));
}