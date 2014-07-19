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

#include "textdialog.h"

#include "gmenu2x.h"
#include "utilities.h"

using namespace std;

TextDialog::TextDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon, vector<string> *text)
	: Dialog(gmenu2x)
{
	this->text = text;
	this->title = title;
	this->description = description;
	this->icon = icon;
	preProcess();
}

void TextDialog::preProcess() {
	unsigned i = 0;

	while (i < text->size()) {
		/* Clean the end of the string, allowing lines that are indented at
		 * the start to stay as such. */
		string line = rtrim(text->at(i));

		if (gmenu2x->font->getTextWidth(line) > (int) gmenu2x->resX - 15) {
			/* At least one full character must fit, in order to advance. */
			size_t fits = 1;
			while (fits < line.length() && !isUTF8Starter(line[fits])) {
				fits++;
			}
			size_t doesntFit = fits;

			/* This preprocessing finds an upper bound on the number of
			 * bytes of full characters that fit on the screen, 2^n, in
			 * n steps. */
			do {
				fits = doesntFit; /* what didn't fit has been determined to fit by a previous iteration */
				doesntFit = min(2 * fits, line.length());
				while (doesntFit < line.length() && !isUTF8Starter(line[doesntFit])) {
					doesntFit++;
				}
			} while (doesntFit <= line.length()
			      && gmenu2x->font->getTextWidth(line.substr(0, doesntFit)) <= (int) gmenu2x->resX - 15);

			/* End this loop when N characters fit but N + 1 don't. */
			while (fits + 1 < doesntFit) {
				size_t guess = fits + (doesntFit - fits) / 2;
				if (!isUTF8Starter(line[guess]))
				{
					size_t oldGuess = guess;
					/* Adjust the guess to the nearest UTF-8 starter that is
					 * not 'fits' or 'doesntFit'. */
					for (size_t offset = 1; offset < (doesntFit - fits) / 2 - 1; offset++) {
						if (isUTF8Starter(line[guess - offset])) {
							guess -= offset;
							break;
						} else if (isUTF8Starter(line[guess + offset])) {
							guess += offset;
							break;
						}
					}
					/* If there's no such character, exit early. */
					if (guess == oldGuess) {
						break;
					}
				}
				if (gmenu2x->font->getTextWidth(line.substr(0, guess)) <= (int) gmenu2x->resX - 15) {
					fits = guess;
				} else {
					doesntFit = guess;
				}
			}

			/* The line shall be split at the last space-separated word that
			 * fully fits, or otherwise at the last character that fits. */
			size_t lastSpace = line.find_last_of(" \t\r", fits);
			if (lastSpace != string::npos) {
				fits = lastSpace;
			}

			/* Insert the rest in a new slot after this line.
			 * TODO (Nebuleon) Don't use a vector for this, because all later
			 * elements are moved, which is inefficient. */
			text->insert(text->begin() + i + 1, ltrim(line.substr(fits)));
			line = rtrim(line.substr(0, fits));
		}

		/* Put the trimmed whole line or the smaller split of the split line
		 * back into the same slot */
		text->at(i) = line;

		i++;
	}
}

void TextDialog::drawText(vector<string> *text, unsigned int y,
		unsigned int firstRow, unsigned int rowsPerPage)
{
	const int fontHeight = gmenu2x->font->getLineSpacing();

	for (unsigned i = firstRow; i < firstRow + rowsPerPage && i < text->size(); i++) {
		const string &line = text->at(i);
		int rowY = y + (i - firstRow) * fontHeight;
		if (line == "----") { // horizontal ruler
			rowY += fontHeight / 2;
			gmenu2x->s->box(5, rowY, gmenu2x->resX - 16, 1, 255, 255, 255, 130);
			gmenu2x->s->box(5, rowY+1, gmenu2x->resX - 16, 1, 0, 0, 0, 130);
		} else {
			gmenu2x->font->write(gmenu2x->s, line, 5, rowY);
		}
	}

	gmenu2x->drawScrollBar(rowsPerPage, text->size(), firstRow);
}

void TextDialog::exec() {
	bool close = false;

	Surface bg(gmenu2x->bg);

	//link icon
	if (!fileExists(icon))
		drawTitleIcon("icons/ebook.png",true,&bg);
	else
		drawTitleIcon(icon,false,&bg);
	writeTitle(title,&bg);
	writeSubTitle(description,&bg);

	gmenu2x->drawButton(&bg, "start", gmenu2x->tr["Exit"],
	gmenu2x->drawButton(&bg, "cancel", "",
	gmenu2x->drawButton(&bg, "down", gmenu2x->tr["Scroll"],
	gmenu2x->drawButton(&bg, "up", "", 5)-10))-10);

	bg.convertToDisplayFormat();

	const int fontHeight = gmenu2x->font->getLineSpacing();
	unsigned int contentY, contentHeight;
	tie(contentY, contentHeight) = gmenu2x->getContentArea();
	const unsigned rowsPerPage = contentHeight / fontHeight;
	contentY += (contentHeight % fontHeight) / 2;

	unsigned firstRow = 0;
	while (!close) {
		bg.blit(gmenu2x->s, 0, 0);
		drawText(text, contentY, firstRow, rowsPerPage);
		gmenu2x->s->flip();

		switch(gmenu2x->input.waitForPressedButton()) {
			case InputManager::UP:
				if (firstRow > 0) firstRow--;
				break;
			case InputManager::DOWN:
				if (firstRow + rowsPerPage < text->size()) firstRow++;
				break;
			case InputManager::ALTLEFT:
				if (firstRow >= rowsPerPage-1) firstRow -= rowsPerPage-1;
				else firstRow = 0;
				break;
			case InputManager::ALTRIGHT:
				if (firstRow + rowsPerPage*2 -1 < text->size()) {
					firstRow += rowsPerPage-1;
				} else {
					firstRow = text->size() < rowsPerPage ?
						0 : text->size() - rowsPerPage;
				}
				break;
			case InputManager::SETTINGS:
			case InputManager::CANCEL:
				close = true;
				break;
			default:
				break;
		}
	}
}
