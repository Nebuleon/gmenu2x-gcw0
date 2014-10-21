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

#ifndef BROWSEDIALOG_H
#define BROWSEDIALOG_H

#include "buttonbox.h"
#include "dialog.h"
#include "filelister.h"
#include "inputmanager.h"

#include <SDL.h>
#include <string>

class OffscreenSurface;

class BrowseDialog : protected Dialog {
public:
	bool exec();

	virtual std::string const& getPath();
	virtual std::string getFile();

protected:
	BrowseDialog(
			GMenu2X *gmenu2x,
			const std::string &title, const std::string &subtitle = "");
	virtual ~BrowseDialog();

	enum Action {
		ACT_NONE,
		ACT_SELECT,
		ACT_CLOSE,
		ACT_UP,
		ACT_DOWN,
		ACT_SCROLLUP,
		ACT_SCROLLDOWN,
		ACT_SCROLLLEFT,
		ACT_SCROLLRIGHT,
		ACT_GOUP,
		ACT_CONFIRM,
	};

	bool close, result;

	/**
	 * Sets the current path of the BrowseDialog to the specified value.
	 * If it is non-empty and does not end with '/', '/' is appended to it.
	 */
	void setPath(std::string const& path);

	FileLister fl;
	unsigned int selected;
	std::string path;

	ButtonBox buttonBox;

	SDL_Rect clipRect;

	unsigned int topBarHeight;
	unsigned int numRows;
	unsigned int rowHeight;

	/**
	 * The first (zero-based) column of pixels to be displayed from the render
	 * of the selected file's name.
	 * The range of valid values for this member variable is 0 to 32767, the
	 * intersection of the ranges of 'unsigned int' and SDL's Sint16 type.
	 */
	unsigned int nameScroll;

	void writePath(Surface& s);

	/**
	 * Initialises the buttonBox member with the buttons used by the dialog.
	 * This method can use the current selection of member variable 'fl', and
	 * is called before paint() in each run through the loop in exec.
	 * The implementation in BrowseDialog clears buttonBox, then creates
	 * buttons for 'Up one folder' (if applicable), 'Select', 'Confirm' and
	 * 'Exit'.
	 */
	virtual void initButtonBox();

	/**
	 * Initialises member icons. The implementation in BrowseDialog creates
	 * iconGoUp, iconFolder and iconFile.
	 */
	virtual void initIcons();

	/**
	 * Initialises the display. The implementation in BrowseDialog fills
	 * clipRect, topBarHeight, numRows and rowHeight.
	 */
	virtual void initDisplay();

	/**
	 * Notifies implementations about changes to the selection. The
	 * implementation in BrowseDialog does nothing.
	 */
	virtual void selectionChanged();

	void centerSelection();
	void adjustSelection();

	void resetNameScroll();

	/**
	 * Scrolls the viewport for the name of the currently selected file
	 * to the left (i.e. more pixels from the left are shown) or
	 * to the right (i.e. more pixels from the right are shown) by one step.
	 */
	void applyNameScroll(bool left);

	/**
	 * Sets the initial path of the file browser.
	 * The implementation in BrowseDialog attempts to resolve the initial path
	 * in a way that most subclasses will find useful.
	 */
	virtual void initPath();

	/**
	 * Sets the initial selection (member variable 'selected').
	 * The implementation in BrowseDialog sets the selection to 0 (the ..
	 * entry, or first file if directories are not shown in 'fl').
	 */
	virtual void initSelection();

	/**
	 * Retrieves the BrowseDialog action corresponding to the given
	 * InputManager button.
	 * The implementation in BrowseDialog should not need to be overridden.
	 * It provides all the necessary mappings.
	 */
	Action getAction(InputManager::Button button);

	/**
	 * Waits for an input, translates it to a BrowseDialog action, then calls
	 * virtual methods to carry out the action in a subclass-specific manner.
	 * The implementation in BrowseDialog should not need to be overridden.
	 */
	void handleInput();

	/**
	 * Paints the background to the display surface.
	 * The implementation in BrowseDialog paints the wallpaper.
	 */
	virtual void paintBackground();

	/**
	 * Paints the icon to the display surface.
	 * The implementation in BrowseDialog paints icons/explorer.png.
	 */
	virtual void paintIcon();

	/**
	 * Paints the entire screen.
	 * The implementation in BrowseDialog calls paintBackground and paintIcon,
	 * then paints the title and subtitle, 'buttonBox', and the icons and
	 * names of visible files and directories.
	 */
	virtual void paint();

	/**
	 * Navigates to the parent directory. May be overridden to do nothing, if
	 * directory browsing is unavailable.
	 * The implementation in BrowseDialog removes the last component of the
	 * path in member variable 'path' and asks member variable 'fl' for its
	 * contents, then tries to select (member variable 'selected') the
	 * previous directory in the parent.
	 */
	virtual void directoryUp();

	/**
	 * Navigates to the directory selected by 'fl[selected]', both member
	 * variables. May be overridden to do nothing, if directory browsing is
	 * unavailable.
	 * The implementation in BrowseDialog adds the newly-selected directory as
	 * a path component in member variable 'path'.
	 */
	virtual void directoryEnter();

	/**
	 * Sets member variables 'close' and 'result' to true to exit the browser.
	 * The user is considered not to have cancelled the dialog.
	 */
	void confirm();

	/**
	 * Sets member variable 'close' to true and 'result' to false to exit the
	 * browser. The user is considered to have cancelled the dialog.
	 */
	void quit();

	/**
	 * Returns true if the entry at 'fl[selected]' can be returned by this
	 * BrowseDialog as its result, or false if it is not the kind of entry
	 * it wants.
	 * The implementation in BrowseDialog accepts files.
	 */
	virtual bool canSelect();

private:
	OffscreenSurface *iconGoUp;
	OffscreenSurface *iconFolder;
	OffscreenSurface *iconFile;

	std::string title;
	std::string subtitle;
	std::string displayedPath;

	unsigned int firstElement;
};

#endif // INPUTDIALOG_H
