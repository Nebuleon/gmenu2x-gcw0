#include "font.h"

#include "debug.h"
#include "surface.h"
#include "utilities.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>

/* TODO: Let the theme choose the font and font size */
#define TTF_FONT "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf"
#define TTF_FONT_SIZE 12

using namespace std;

Font *Font::defaultFont()
{
	return new Font(TTF_FONT, TTF_FONT_SIZE);
}

Font::Font(const std::string &path, unsigned int size)
{
	font = nullptr;
	lineSpacing = 1;

	/* Note: TTF_Init and TTF_Quit perform reference counting, so call them
	 * both unconditionally for each font. */
	if (TTF_Init() < 0) {
		ERROR("Unable to init SDL_ttf library\n");
		return;
	}

	font = TTF_OpenFont(path.c_str(), size);
	if (!font) {
		ERROR("Unable to open font\n");
		TTF_Quit();
		return;
	}

	lineSpacing = TTF_FontLineSkip(font);
}

Font::~Font()
{
	if (font) {
		TTF_CloseFont(font);
		TTF_Quit();
	}
}

int Font::getTextWidth(const string &text)
{
	if (!font) {
		return 1;
	}

	int w;
	size_t pos = text.find('\n', 0);
	if (pos == string::npos) {
		TTF_SizeUTF8(font, text.c_str(), &w, nullptr);
		return w;
	} else {
		int maxWidth = 1;
		size_t prev = 0;
		do {
			TTF_SizeUTF8(font, text.substr(prev, pos - prev).c_str(), &w, nullptr);
			maxWidth = max(w, maxWidth);
			prev = pos + 1;
			pos = text.find('\n', prev);
		} while (pos != string::npos);
		TTF_SizeUTF8(font, text.substr(prev).c_str(), &w, nullptr);
		return max(w, maxWidth);
	}
}

int Font::getTextHeight(const string &text)
{
	int nLines = 1;
	size_t pos = 0;
	while ((pos = text.find('\n', pos)) != string::npos) {
		nLines++;
		pos++;
	}
	return nLines * getLineSpacing();
}

void Font::write(Surface *surface, const string &text,
			int x, int y, HAlign halign, VAlign valign)
{
	if (!font) {
		return;
	}

	size_t pos = text.find('\n', 0);
	if (pos == string::npos) {
		writeLine(surface, text, x, y, halign, valign);
	} else {
		size_t prev = 0;
		do {
			writeLine(surface, text.substr(prev, pos - prev),
					x, y, halign, valign);
			y += lineSpacing;
			prev = pos + 1;
			pos = text.find('\n', prev);
		} while (pos != string::npos);
		writeLine(surface, text.substr(prev), x, y, halign, valign);
	}
}

void Font::writeLine(Surface *surface, std::string const& text,
				int x, int y, HAlign halign, VAlign valign)
{
	if (!font) {
		return;
	}
	if (text.empty()) {
		// SDL_ttf will return a nullptr when rendering the empty string.
		return;
	}

	switch (valign) {
	case VAlignTop:
		break;
	case VAlignMiddle:
		y -= lineSpacing / 2;
		break;
	case VAlignBottom:
		y -= lineSpacing;
		break;
	}

	SDL_Color color = { 0, 0, 0, 0 };
	SDL_Surface *s = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if (!s) {
		ERROR("Font rendering failed for text \"%s\"\n", text.c_str());
		return;
	}

	switch (halign) {
	case HAlignLeft:
		break;
	case HAlignCenter:
		x -= s->w / 2;
		break;
	case HAlignRight:
		x -= s->w;
		break;
	}

	SDL_Rect rect = { (Sint16) x, (Sint16) (y - 1), 0, 0 };
	SDL_BlitSurface(s, NULL, surface->raw, &rect);

	/* Note: rect.x / rect.y are reset everytime because SDL_BlitSurface
	 * will modify them if negative */
	rect.x = x;
	rect.y = y + 1;
	SDL_BlitSurface(s, NULL, surface->raw, &rect);

	rect.x = x - 1;
	rect.y = y;
	SDL_BlitSurface(s, NULL, surface->raw, &rect);

	rect.x = x + 1;
	rect.y = y;
	SDL_BlitSurface(s, NULL, surface->raw, &rect);
	SDL_FreeSurface(s);

	rect.x = x;
	rect.y = y;
	color.r = 0xff;
	color.g = 0xff;
	color.b = 0xff;

	s = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if (!s) {
		ERROR("Font rendering failed for text \"%s\"\n", text.c_str());
		return;
	}
	SDL_BlitSurface(s, NULL, surface->raw, &rect);
	SDL_FreeSurface(s);
}
