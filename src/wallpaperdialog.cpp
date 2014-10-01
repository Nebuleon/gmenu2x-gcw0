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

#include "wallpaperdialog.h"

#include "debug.h"

#include <algorithm>
#include <iostream>

using namespace std;

WallpaperDialog::WallpaperDialog(GMenu2X *gmenu2x)
	: BrowseDialog(gmenu2x,
			gmenu2x->tr["Wallpaper selection"],
			gmenu2x->tr["Select a wallpaper from the list"])
{
	fl.setShowDirectories(false);
	fl.setFilter("png");
}

void WallpaperDialog::initPath()
{
	fl.browse(GMenu2X::getHome() + "/skins/"
		+ gmenu2x->confStr["skin"] + "/wallpapers", true);
	fl.browse(GMENU2X_SYSTEM_DIR "/skins/"
		+ gmenu2x->confStr["skin"] + "/wallpapers", false);

	if (gmenu2x->confStr["skin"] != "Default") {
		fl.browse(GMenu2X::getHome() + "/skins/Default/wallpapers", false);
		fl.browse(GMENU2X_SYSTEM_DIR "/skins/Default/wallpapers", false);
	}

	DEBUG("Wallpapers: %i\n", fl.size());
}

void WallpaperDialog::initSelection()
{
	BrowseDialog::initSelection();

	if (!initialWallpaper.empty()) {
		string::size_type pos = initialWallpaper.rfind("/");
		if (pos != string::npos)
			initialWallpaper = initialWallpaper.substr(pos + 1);

		auto& wallpapers = fl.getFiles();

		auto it = find(wallpapers.begin(), wallpapers.end(), initialWallpaper);
		if (it != wallpapers.end()) {
			selected = it - wallpapers.begin();
		}
	}
}

void WallpaperDialog::paintBackground()
{
	// Preview the new wallpaper.
	gmenu2x->sc[((string)"skin:wallpapers/" + fl[selected]).c_str()]
			->blit(*gmenu2x->s, 0, 0);

	gmenu2x->drawTopBar(*gmenu2x->s);
	gmenu2x->drawBottomBar(*gmenu2x->s);
}

bool WallpaperDialog::exec(string currentWallpaper)
{
	initialWallpaper = currentWallpaper;

	bool result = BrowseDialog::exec();

	for (size_t i = 0; i < fl.size(); i++) {
		gmenu2x->sc.del("skin:wallpapers/" + fl[i]);
	}

	return result;
}

string WallpaperDialog::getFullPath()
{
	return gmenu2x->sc.getSkinFilePath("wallpapers/" + fl[selected]);
}
