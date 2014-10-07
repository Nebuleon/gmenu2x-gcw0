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

#ifndef WALLPAPERDIALOG_H
#define WALLPAPERDIALOG_H

#include "browsedialog.h"

#include <string>

class WallpaperDialog : public BrowseDialog {
public:
	WallpaperDialog(GMenu2X *gmenu2x);

	bool exec(std::string currentWallpaper);

	std::string getFullPath();

protected:
	virtual void initPath() override;
	virtual void initSelection() override;
	virtual void paintBackground() override;

private:
	std::string initialWallpaper;
};

#endif // WALLPAPERDIALOG_H
