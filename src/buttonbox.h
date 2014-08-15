#ifndef __BUTTONBOX_H__
#define __BUTTONBOX_H__

#include "iconbutton.h"

#include <memory>
#include <vector>

class GMenu2X;
class Surface;

class ButtonBox
{
public:
	ButtonBox(GMenu2X *gmenu2x);

	void add(std::unique_ptr<IconButton> button);
	void clear();

	void paint(Surface& s, unsigned int x);
	void handleTS();

private:
	std::vector<std::unique_ptr<IconButton> > buttons;
	GMenu2X *gmenu2x;
};

#endif
