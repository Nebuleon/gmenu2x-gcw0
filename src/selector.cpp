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

#include <algorithm>

using std::string;

Selector::Selector(GMenu2X *gmenu2x, LinkApp& link, const string &selectorDir)
	: BrowseDialog(gmenu2x, link.getTitle(), link.getDescription())
	, link(link)
{
	setPath(selectorDir.empty() ? link.getSelectorDir() : selectorDir);
	fl.setShowDirectories(link.getSelectorBrowser());
	fl.setFilter(link.getSelectorFilter());
}

void Selector::selectionChanged()
{
	preview.reset();

	if (selected < fl.size() && fl.isFile(selected)) {
		string fileName = trimExtension(fl[selected]) + ".png";
		std::unique_ptr<OffscreenSurface> screenshot = OffscreenSurface::loadImage(
				getPath() + "previews/" + fileName, false);
		if (!screenshot) {
			screenshot = OffscreenSurface::loadImage(
					getPath() + ".previews/" + fileName, false);
		}
		if (screenshot) {
			screenshot = screenshot->scaleToFit(gmenu2x->resX, gmenu2x->resY);
		}
		preview = std::move(screenshot);
	}
}

void Selector::paintBackground()
{
	BrowseDialog::paintBackground();

	if (preview) {
		preview->blitRight(*gmenu2x->s, 320, 0, 320, 240, 128u);
	}
}

void Selector::paintIcon()
{
	drawTitleIcon(*gmenu2x->s, link.getIconPath(), true);
}

void Selector::initSelection()
{
	BrowseDialog::initSelection();

	if (!fileHint.empty()) {
		case_less comparator;

		auto& files = fl.getFiles();
		/* Get the start of a range of file entries having the same name,
		 * case-insensitive, as the hint. */
		auto it = lower_bound(files.begin(), files.end(), fileHint, comparator);

		if (it != files.end()) {
			/* Look for a file having the same name, case-sensitive, up until
			 * the last file having the same name, case-insensitive. */
			for (auto itCS = it;
					itCS != files.end() && !comparator(fileHint, *itCS);
					++itCS) {
				if (*itCS == fileHint) {
					it = itCS;
					break;
				}
			}
			selected = fl.dirCount() + (it - files.begin());
		} else if (fl.size() > 0) {
			/* If we get here, then the file hint comes after the name of
			 * the last file in the list, case-insensitive (and there is
			 * at least one file). */
			selected = fl.size() - 1;
		}
	}
}

bool Selector::exec(string fileHint) {
	this->fileHint = fileHint;

	return BrowseDialog::exec();
}
