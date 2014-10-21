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

#ifndef SELECTOR_H
#define SELECTOR_H

#include "browsedialog.h"
#include "surface.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class LinkApp;
class FileLister;

class Selector : public BrowseDialog {
public:
	Selector(GMenu2X *gmenu2x, LinkApp& link,
			const std::string &selectorDir = "");

	/*
	 * Executes the selector, using fileHint to initially select a file.
	 * If fileHint matches the name of a file exactly, then that file is
	 * selected. Otherwise, the nearest file, in case-insensitive order,
	 * is selected.
	 */
	bool exec(std::string fileHint = "");

protected:
	virtual void initSelection() override;
	virtual void selectionChanged() override;
	virtual void paintBackground() override;
	virtual void paintIcon() override;

private:
	LinkApp& link;

	std::unique_ptr<OffscreenSurface> preview;

	std::string fileHint;
};

#endif // SELECTOR_H
