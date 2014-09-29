#include "browsedialog.h"

#include <algorithm>

#include "filelister.h"
#include "gmenu2x.h"
#include "iconbutton.h"
#include "surface.h"
#include "utilities.h"

using std::bind;
using std::string;
using std::unique_ptr;
using std::tie;
using std::min;
using std::max;

BrowseDialog::BrowseDialog(
		GMenu2X *gmenu2x, Touchscreen &ts_,
		const string &title, const string &subtitle)
	: Dialog(gmenu2x)
	, ts(ts_)
	, title(title)
	, subtitle(subtitle)
	, ts_pressed(false)
{
}

BrowseDialog::~BrowseDialog()
{
}

void BrowseDialog::initButtonBox()
{
	buttonBox.clear();

	// Cancel to go up a directory, if directories are shown.
	// Accept also goes up a directory if the selection is "..".
	if (fl.getShowDirectories() && getPath() != "/") {
		if (selected < fl.size() && fl[selected] == "..") {
			buttonBox.add(unique_ptr<IconButton>(new IconButton(
					gmenu2x, ts, "skin:imgs/buttons/accept.png")));
		}
		buttonBox.add(unique_ptr<IconButton>(new IconButton(
				gmenu2x, ts, "skin:imgs/buttons/cancel.png",
				gmenu2x->tr["Up one folder"],
				bind(&BrowseDialog::directoryUp, this))));
	}

	// Accept to enter a directory (as opposed to selecting it). -else-
	// Accept to confirm the selection of a file, if files are allowed.
	if (selected < fl.size() && fl.isDirectory(selected) && fl[selected] != "..") {
		buttonBox.add(unique_ptr<IconButton>(new IconButton(
				gmenu2x, ts, "skin:imgs/buttons/accept.png",
				gmenu2x->tr["Enter"],
				bind(&BrowseDialog::directoryEnter, this))));
	} else if (canSelect()) {
		buttonBox.add(unique_ptr<IconButton>(new IconButton(
				gmenu2x, ts, "skin:imgs/buttons/accept.png")));
	}

	// Start to confirm the selection of a file or directory if allowed.
	if (canSelect()) {
		buttonBox.add(unique_ptr<IconButton>(new IconButton(
				gmenu2x, ts, "skin:imgs/buttons/start.png",
				gmenu2x->tr["Select"],
				bind(&BrowseDialog::confirm, this))));
	}

	// Cancel (if directories are hidden) or Select to exit.
	if (!fl.getShowDirectories()) {
		buttonBox.add(unique_ptr<IconButton>(new IconButton(
				gmenu2x, ts, "skin:imgs/buttons/cancel.png")));
	}
	buttonBox.add(unique_ptr<IconButton>(new IconButton(
			gmenu2x, ts, "skin:imgs/buttons/select.png",
			gmenu2x->tr["Exit"],
			bind(&BrowseDialog::quit, this))));
}

void BrowseDialog::initIcons()
{
	iconGoUp = gmenu2x->sc.skinRes("imgs/go-up.png");
	iconFolder = gmenu2x->sc.skinRes("imgs/folder.png");
	iconFile = gmenu2x->sc.skinRes("imgs/file.png");
}

void BrowseDialog::initDisplay()
{
	unsigned int top, height;
	unsigned int lineSpacing = gmenu2x->font->getLineSpacing();
	tie(top, height) = gmenu2x->getContentArea();
	// Reserve space for the path display, if there's a path.
	if (!getPath().empty()) {
		top += lineSpacing;
		height -= lineSpacing;
	}

	// Figure out how many items we can fit in the content area.
	rowHeight = lineSpacing;
	if (fl.getShowDirectories() && iconFolder) {
		rowHeight = max(rowHeight, (unsigned int) (iconFolder->height() + 2));
	}
	numRows = max(height / rowHeight, 1u);
	// Redistribute any leftover space.
	rowHeight = height / numRows;
	topBarHeight = top + (height - rowHeight * numRows) / 2;

	clipRect = (SDL_Rect) {
		0,
		static_cast<Sint16>(topBarHeight + 1),
		static_cast<Uint16>(gmenu2x->resX - 9),
		static_cast<Uint16>(height - 1)
	};
	touchRect = (SDL_Rect) {
		2,
		static_cast<Sint16>(topBarHeight + 4),
		static_cast<Uint16>(gmenu2x->resX - 12),
		clipRect.h
	};
}

void BrowseDialog::initPath()
{
	string path = getPath();
	if (path.empty()) {
		path = CARD_ROOT;
	}
	// fl.browse has to run at least once.
	while (!fl.browse(path) && fl.getShowDirectories() && path != "/") {
		// The given directory could not be opened; try parent.
		path = parentDir(path);
	}
	setPath(path);
}

void BrowseDialog::initSelection()
{
	selected = 0;
}

bool BrowseDialog::exec()
{
	initIcons();
	initDisplay();

	initPath();
	initSelection();

	close = false;
	while (!close) {
		initButtonBox();
		if (ts.available()) ts.poll();

		paint();

		handleInput();
	}

	return result;
}

BrowseDialog::Action BrowseDialog::getAction(InputManager::Button button)
{
	switch (button) {
		case InputManager::MENU:
			return BrowseDialog::ACT_CLOSE;
		case InputManager::UP:
			return BrowseDialog::ACT_UP;
		case InputManager::DOWN:
			return BrowseDialog::ACT_DOWN;
		case InputManager::ALTLEFT:
			return BrowseDialog::ACT_SCROLLUP;
		case InputManager::ALTRIGHT:
			return BrowseDialog::ACT_SCROLLDOWN;
		case InputManager::CANCEL:
			return BrowseDialog::ACT_GOUP;
		case InputManager::ACCEPT:
			return BrowseDialog::ACT_SELECT;
		case InputManager::SETTINGS:
			return BrowseDialog::ACT_CONFIRM;
		default:
			return BrowseDialog::ACT_NONE;
	}
}

void BrowseDialog::handleInput()
{
	InputManager::Button button = gmenu2x->input.waitForPressedButton();

	BrowseDialog::Action action;
	if (ts_pressed && !ts.pressed()) {
		action = BrowseDialog::ACT_SELECT;
		ts_pressed = false;
	} else {
		action = getAction(button);
	}

	if (ts.available() && ts.pressed() && !ts.inRect(touchRect)) {
		ts_pressed = false;
	}

	// Various fixups.
	if (action == BrowseDialog::ACT_SELECT
	 && selected < fl.size() && fl[selected] == "..") {
		action = BrowseDialog::ACT_GOUP;
	}

	switch (action) {
	case BrowseDialog::ACT_UP:
		if (fl.size() > 0) {
			selected = (selected == 0)
					? fl.size() - 1
					: selected - 1;
		}
		break;
	case BrowseDialog::ACT_SCROLLUP:
		if (fl.size() > 0) {
			selected = (selected <= numRows - 2)
					? 0
					: selected - (numRows - 2);
		}
		break;
	case BrowseDialog::ACT_DOWN:
		if (fl.size() > 0) {
			selected = (fl.size() - 1 <= selected)
					? 0
					: selected + 1;
		}
		break;
	case BrowseDialog::ACT_SCROLLDOWN:
		if (fl.size() > 0) {
			selected = (selected + (numRows - 2) >= fl.size())
					? fl.size() - 1
					: selected + (numRows - 2);
		}
		break;
	case BrowseDialog::ACT_GOUP:
		if (fl.getShowDirectories()) {
			directoryUp();
			break;
		}
		/* Fall through (If not showing directories, GOUP acts as CLOSE) */
	case BrowseDialog::ACT_CLOSE:
		quit();
		break;
	case BrowseDialog::ACT_SELECT:
		if (selected < fl.size() && fl.isDirectory(selected)) {
			directoryEnter();
			break;
		}
		/* Fall through (If not a directory, SELECT acts as CONFIRM) */
	case BrowseDialog::ACT_CONFIRM:
		if (selected < fl.size() && canSelect()) {
			confirm();
		}
		break;
	default:
		break;
	}

	buttonBox.handleTS();
}

#include <iostream>

void BrowseDialog::directoryUp()
{
	string oldDir = getPath();
	if (oldDir != "/") {
		string newDir = parentDir(oldDir);
		setPath(newDir);
		fl.browse(getPath());
		// Find the position of the previous directory among the directories of
		// the parent. If it's not found, select 0.
		string oldName = oldDir.substr(newDir.size(), oldDir.size() - newDir.size() - 1);
		auto& subdirs = fl.getDirectories();

		auto it = find(subdirs.begin(), subdirs.end(), oldName);
		selected = it == subdirs.end() ? 0 : it - subdirs.begin();
	}
}

void BrowseDialog::directoryEnter()
{
	string oldDir = getPath();
	if (oldDir[oldDir.size()-1] != '/') {
		oldDir += "/";
	}

	string newDir = oldDir + fl[selected] + "/";
	setPath(newDir);

	selected = 0;
	fl.browse(getPath());
}

void BrowseDialog::confirm()
{
	result = true;
	close = true;
}

void BrowseDialog::quit()
{
	result = false;
	close = true;
}

void BrowseDialog::paintBackground()
{
	gmenu2x->bg->blit(*gmenu2x->s, 0, 0);
}

void BrowseDialog::paintIcon()
{
	drawTitleIcon(*gmenu2x->s, "icons/explorer.png", true);
}

void BrowseDialog::paint()
{
	OutputSurface& s = *gmenu2x->s;

	unsigned int i, iY;
	unsigned int firstElement, lastElement;
	unsigned int offsetY;

	paintBackground();
	paintIcon();
	writeTitle(*gmenu2x->s, title);
	writeSubTitle(*gmenu2x->s, subtitle);
	buttonBox.paint(*gmenu2x->s, 5, gmenu2x->resY - 1);

	if (fl.size() <= numRows || selected <= numRows / 2) {
		firstElement = 0;
		lastElement = min(fl.size(), numRows);
	} else {
		lastElement = min(fl.size(), selected + (numRows - numRows / 2));
		firstElement = lastElement - numRows;
	}

	// Path display (you are here)
	string path = getPath();
	if (!path.empty()) {
		gmenu2x->font->write(s, path, 5, topBarHeight,
				Font::HAlignLeft, Font::VAlignBottom);
	}

	offsetY = topBarHeight + 1;

	//Files & Directories
	s.setClipRect(clipRect);
	if (fl.size() == 0) {
		gmenu2x->font->write(s, "(" + gmenu2x->tr["no items"] + ")",
				4, topBarHeight + rowHeight / 2,
				Font::HAlignLeft, Font::VAlignMiddle);
	} else {
		//Selection
		iY = topBarHeight + 1 + (selected - firstElement) * rowHeight;
		s.box(2, iY, gmenu2x->resX - 12, rowHeight - 1,
				gmenu2x->skinConfColors[COLOR_SELECTION_BG]);

		const int nameX = fl.getShowDirectories() ? 24 : 5;
		for (i = firstElement; i < lastElement; i++) {
			if (fl.getShowDirectories()) {
				Surface *icon;
				if (fl.isDirectory(i)) {
					if (fl[i] == "..") {
						icon = iconGoUp;
					} else {
						icon = iconFolder;
					}
				} else {
					icon = iconFile;
				}
				if (icon) {
					icon->blit(s, 5, offsetY);
				}
			}
			gmenu2x->font->write(s, fl[i], nameX, offsetY + rowHeight / 2,
					Font::HAlignLeft, Font::VAlignMiddle);

			if (ts.available() && ts.pressed()
					&& ts.inRect(touchRect.x, offsetY + 3, touchRect.w, rowHeight)) {
				ts_pressed = true;
				selected = i;
			}

			offsetY += rowHeight;
		}
	}
	s.clearClipRect();

	gmenu2x->drawScrollBar(numRows, fl.size(), firstElement);
	s.flip();
}

const string& BrowseDialog::getPath() {
	return path;
}

void BrowseDialog::setPath(const std::string &path)
{
	if (path.empty() || path[path.length() - 1] == '/') {
		this->path = path;
	} else {
		this->path = path + "/";
	}
}

string BrowseDialog::getFile() {
	return fl[selected];
}

bool BrowseDialog::canSelect() {
	return selected < fl.size() && fl.isFile(selected);
}
