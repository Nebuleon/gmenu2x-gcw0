/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo                           *
 *   massimiliano.torromeo@gmail.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "menusettingrgba.h"

#include "gmenu2x.h"
#include "iconbutton.h"
#include "surface.h"
#include "utilities.h"

#include <sstream>

using std::string;
using std::stringstream;

constexpr unsigned int COMPONENT_WIDTH = 36;

MenuSettingRGBA::MenuSettingRGBA(
		GMenu2X *gmenu2x, Touchscreen &ts_,
		const string &name, const string &description, RGBAColor *value)
	: MenuSetting(gmenu2x, name, description)
	, ts(ts_)
{
	edit = false;

	selPart = 0;
	_value = value;
	originalValue = *value;
	this->setR(this->value().r);
	this->setG(this->value().g);
	this->setB(this->value().b);
	this->setA(this->value().a);

	updateButtonBox();
}

void MenuSettingRGBA::draw(int valueX, int y, int h) {
	MenuSetting::draw(valueX, y, h);
	gmenu2x->s->rectangle( valueX, y + 1, h - 2, h - 2, 0,0,0,255 );
	gmenu2x->s->rectangle( valueX + 1, y + 2, h - 4, h - 4, 255,255,255,255 );
	gmenu2x->s->box( valueX + 2, y + 3, h - 6, h - 6, value() );
	gmenu2x->s->write( gmenu2x->font, "R: "+strR, valueX + h + 3, y, Font::HAlignLeft, Font::VAlignTop );
	gmenu2x->s->write( gmenu2x->font, "G: "+strG, valueX + h + 3 + COMPONENT_WIDTH, y, Font::HAlignLeft, Font::VAlignTop );
	gmenu2x->s->write( gmenu2x->font, "B: "+strB, valueX + h + 3 + COMPONENT_WIDTH * 2, y, Font::HAlignLeft, Font::VAlignTop );
	gmenu2x->s->write( gmenu2x->font, "A: "+strA, valueX + h + 3 + COMPONENT_WIDTH * 3, y, Font::HAlignLeft, Font::VAlignTop );
}

void MenuSettingRGBA::handleTS(int valueX, int y, int h) {
	if (ts.pressed()) {
		for (int i=0; i<4; i++) {
			if (i!=selPart && ts.inRect(valueX + h + i * COMPONENT_WIDTH,y,COMPONENT_WIDTH,h)) {
				selPart = i;
				break;
			}
		}
	}

	MenuSetting::handleTS(valueX, y, h);
}

bool MenuSettingRGBA::handleButtonPress(InputManager::Button button)
 {
	if (edit) {
		switch (button) {
			case InputManager::LEFT:
				dec();
				break;
			case InputManager::RIGHT:
				inc();
				break;
			case InputManager::ALTLEFT:
				update_value(-10);
				break;
			case InputManager::ALTRIGHT:
				update_value(10);
				break;
			case InputManager::ACCEPT:
			case InputManager::UP:
			case InputManager::DOWN:
				edit = false;
				updateButtonBox();
				break;
			default:
				return false;
		}
	} else {
		switch (button) {
			case InputManager::LEFT:
				leftComponent();
				break;
			case InputManager::RIGHT:
				rightComponent();
				break;
			case InputManager::ACCEPT:
				edit = true;
				updateButtonBox();
				break;
			default:
				return false;
		}
	}
	return true;
}

void MenuSettingRGBA::update_value(int value)
{
	setSelPart(constrain(getSelPart() + value, 0, 255));
}

void MenuSettingRGBA::dec()
{
	update_value(-1);
}

void MenuSettingRGBA::inc()
{
	update_value(+1);
}

void MenuSettingRGBA::leftComponent()
{
	selPart = constrain(selPart-1,0,3);
}

void MenuSettingRGBA::rightComponent()
{
	selPart = constrain(selPart+1,0,3);
}

void MenuSettingRGBA::setR(unsigned short r)
{
	_value->r = r;
	stringstream ss;
	ss << r;
	ss >> strR;
}

void MenuSettingRGBA::setG(unsigned short g)
{
	_value->g = g;
	stringstream ss;
	ss << g;
	ss >> strG;
}

void MenuSettingRGBA::setB(unsigned short b)
{
	_value->b = b;
	stringstream ss;
	ss << b;
	ss >> strB;
}

void MenuSettingRGBA::setA(unsigned short a)
{
	_value->a = a;
	stringstream ss;
	ss << a;
	ss >> strA;
}

void MenuSettingRGBA::setSelPart(unsigned short value)
{
	switch (selPart) {
		default: case 0: setR(value); break;
		case 1: setG(value); break;
		case 2: setB(value); break;
		case 3: setA(value); break;
	}
}

RGBAColor MenuSettingRGBA::value()
{
	return *_value;
}

unsigned short MenuSettingRGBA::getSelPart()
{
	switch (selPart) {
		default: case 0: return value().r;
		case 1: return value().g;
		case 2: return value().b;
		case 3: return value().a;
	}
}

void MenuSettingRGBA::drawSelected(int valueX, int y, int h)
{
	int x = valueX + selPart * COMPONENT_WIDTH;
	gmenu2x->s->box( x + h, y, COMPONENT_WIDTH, h, gmenu2x->skinConfColors[COLOR_SELECTION_BG] );

	MenuSetting::drawSelected(valueX, y, h);
}

bool MenuSettingRGBA::edited()
{
	return originalValue.r != value().r || originalValue.g != value().g || originalValue.b != value().b || originalValue.a != value().a;
}

void MenuSettingRGBA::updateButtonBox()
{
	buttonBox.clear();
	if (edit) {
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/l.png"));
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/left.png", gmenu2x->tr["Decrease"]));
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/r.png"));
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/right.png", gmenu2x->tr["Increase"]));
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/accept.png", gmenu2x->tr["Confirm"]));
	} else {
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/left.png"));
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/right.png", gmenu2x->tr["Change color component"]));
		buttonBox.add(new IconButton(gmenu2x, ts, "skin:imgs/buttons/accept.png", gmenu2x->tr["Edit"]));
	}
}
