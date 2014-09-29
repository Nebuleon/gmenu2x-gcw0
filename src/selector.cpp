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

#include "selector.h"

#include "gmenu2x.h"
#include "debug.h"
#include "linkapp.h"

using std::string;

Selector::Selector(GMenu2X *gmenu2x, LinkApp& link, const string &selectorDir)
	: BrowseDialog(gmenu2x, link.getTitle(), link.getDescription())
	, link(link)
{
	setPath(selectorDir.empty() ? link.getSelectorDir() : selectorDir);
	fl.setShowDirectories(link.getSelectorBrowser());
	fl.setFilter(link.getSelectorFilter());
}

void Selector::paintBackground()
{
	BrowseDialog::paintBackground();

	if (selected < fl.size() && fl.isFile(selected)) {
		string previewDir = getPath() + "previews/";
		string previewFile = previewDir + trimExtension(fl[selected]) + ".png";
		auto screenshot = OffscreenSurface::loadImage(previewFile, false);
		if (screenshot) {
			screenshot->blitRight(*gmenu2x->s, 320, 0, 320, 240, 128u);
		}
	}
}

void Selector::paintIcon()
{
	drawTitleIcon(*gmenu2x->s, link.getIconPath(), true);
}

void Selector::initSelection()
{
	// Leave the selection as exec puts it.
}

int Selector::exec(int initialSelection) {
	selected = initialSelection;

	return BrowseDialog::exec() ? (int) selected : -1;
}
