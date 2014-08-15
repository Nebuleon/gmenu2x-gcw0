#include "buttonbox.h"

#include "gmenu2x.h"
#include "iconbutton.h"

ButtonBox::ButtonBox(GMenu2X *gmenu2x) : gmenu2x(gmenu2x)
{
}

ButtonBox::~ButtonBox()
{
	clear();
}

void ButtonBox::add(IconButton *button)
{
	buttons.push_back(button);
}

void ButtonBox::clear()
{
	buttons.clear();
}

void ButtonBox::paint(Surface& s, unsigned int x)
{
	const int y = gmenu2x->resY - 1;
	for (auto button : buttons) {
		auto rect = button->getRect();
		button->setPosition(x, y - rect.h);
		button->paint(s);
		x += button->getRect().w + 6;
	}
}

void ButtonBox::handleTS()
{
	for (auto button : buttons) {
		button->handleTS();
	}
}
