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

#include "surface.h"

#include "debug.h"
#include "imageio.h"
#include "utilities.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <utility>

using namespace std;


// RGBAColor:

RGBAColor RGBAColor::fromString(const string &strColor) {
	return {
		uint8_t(constrain(strtol(strColor.substr(0, 2).c_str(), nullptr, 16),
		                  0, 255)),
		uint8_t(constrain(strtol(strColor.substr(2, 2).c_str(), nullptr, 16),
		                  0, 255)),
		uint8_t(constrain(strtol(strColor.substr(4, 2).c_str(), nullptr, 16),
		                  0, 255)),
		uint8_t(constrain(strtol(strColor.substr(6, 2).c_str(), nullptr, 16),
		                  0, 255)),
	};
}

ostream& operator<<(ostream& os, RGBAColor const& color) {
	auto oldfill = os.fill('0');
	auto oldflags = os.setf(ios::hex | ios::right,
	                        ios::basefield | ios::adjustfield);
	os << setw(2) << uint32_t(color.r)
	   << setw(2) << uint32_t(color.g)
	   << setw(2) << uint32_t(color.b)
	   << setw(2) << uint32_t(color.a);
	os.fill(oldfill);
	os.setf(oldflags);
	return os;
}


// Surface:

Surface::Surface(Surface const& other)
	: Surface(SDL_ConvertSurface(other.raw, other.raw->format, SDL_SWSURFACE))
{
	// Note: A bug in SDL_ConvertSurface() leaves the per-surface alpha
	//       undefined when converting from RGBA to RGBA. This can cause
	//       problems if the surface is later converted to a format without
	//       an alpha channel, such as the display format.
	raw->format->alpha = other.raw->format->alpha;
}

void Surface::blit(SDL_Surface *destination, int x, int y, int w, int h, int a) const {
	if (destination == NULL || a==0) return;

	SDL_Rect src = { 0, 0, static_cast<Uint16>(w), static_cast<Uint16>(h) };
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	if (a>0 && a!=raw->format->alpha)
		SDL_SetAlpha(raw, SDL_SRCALPHA|SDL_RLEACCEL, a);
	SDL_BlitSurface(raw, (w==0 || h==0) ? NULL : &src, destination, &dest);
}
void Surface::blit(Surface& destination, int x, int y, int w, int h, int a) const {
	blit(destination.raw, x, y, w, h, a);
}

void Surface::blitCenter(SDL_Surface *destination, int x, int y, int w, int h, int a) const {
	int ow = raw->w / 2; if (w != 0) ow = min(ow, w / 2);
	int oh = raw->h / 2; if (h != 0) oh = min(oh, h / 2);
	blit(destination, x - ow, y - oh, w, h, a);
}
void Surface::blitCenter(Surface& destination, int x, int y, int w, int h, int a) const {
	blitCenter(destination.raw, x, y, w, h, a);
}

void Surface::blitRight(SDL_Surface *destination, int x, int y, int w, int h, int a) const {
	if (!w) w = raw->w;
	blit(destination, x - min(raw->w, w), y, w, h, a);
}
void Surface::blitRight(Surface& destination, int x, int y, int w, int h, int a) const {
	if (!w) w = raw->w;
	blitRight(destination.raw, x, y, w, h, a);
}

void Surface::box(SDL_Rect re, RGBAColor c) {
	if (c.a == 255) {
		SDL_FillRect(raw, &re, c.pixelValue(raw->format));
	} else if (c.a != 0) {
		fillRectAlpha(re, c);
	}
}

void Surface::rectangle(SDL_Rect re, RGBAColor c) {
	if (re.h >= 1) {
		// Top.
		box(SDL_Rect { re.x, re.y, re.w, 1 }, c);
	}
	if (re.h >= 2) {
		Sint16 ey = re.y + re.h - 1;
		// Bottom.
		box(SDL_Rect { re.x, ey, re.w, 1 }, c);

		Sint16 ex = re.x + re.w - 1;
		Sint16 sy = re.y + 1;
		Uint16 sh = re.h - 2;
		// Left.
		if (re.w >= 1) {
			box(SDL_Rect { re.x, sy, 1, sh }, c);
		}
		// Right.
		if (re.w >= 2) {
			box(SDL_Rect { ex, sy, 1, sh }, c);
		}
	}
}

void Surface::clearClipRect() {
	SDL_SetClipRect(raw,NULL);
}

void Surface::setClipRect(int x, int y, int w, int h) {
	SDL_Rect rect = {
		static_cast<Sint16>(x), static_cast<Sint16>(y),
		static_cast<Uint16>(w), static_cast<Uint16>(h)
	};
	setClipRect(rect);
}

void Surface::setClipRect(SDL_Rect rect) {
	SDL_SetClipRect(raw,&rect);
}

void Surface::applyClipRect(SDL_Rect& rect) {
	SDL_Rect clip;
	SDL_GetClipRect(raw, &clip);

	// Clip along X-axis.
	if (rect.x < clip.x) {
		rect.w = max(rect.x + rect.w - clip.x, 0);
		rect.x = clip.x;
	}
	if (rect.x + rect.w > clip.x + clip.w) {
		rect.w = max(clip.x + clip.w - rect.x, 0);
	}

	// Clip along Y-axis.
	if (rect.y < clip.y) {
		rect.h = max(rect.y + rect.h - clip.y, 0);
		rect.y = clip.y;
	}
	if (rect.y + rect.h > clip.y + clip.h) {
		rect.h = max(clip.y + clip.h - rect.y, 0);
	}
}

void Surface::blit(Surface& destination, SDL_Rect container, Font::HAlign halign, Font::VAlign valign) const {
	switch (halign) {
	case Font::HAlignLeft:
		break;
	case Font::HAlignCenter:
		container.x += container.w / 2 - raw->w / 2;
		break;
	case Font::HAlignRight:
		container.x += container.w-raw->w;
		break;
	}

	switch (valign) {
	case Font::VAlignTop:
		break;
	case Font::VAlignMiddle:
		container.y += container.h / 2 - raw->h / 2;
		break;
	case Font::VAlignBottom:
		container.y += container.h-raw->h;
		break;
	}

	blit(destination, container.x, container.y);
}

static inline uint32_t mult8x4(uint32_t c, uint8_t a) {
	return ((((c >> 8) & 0x00FF00FF) * a) & 0xFF00FF00)
	     | ((((c & 0x00FF00FF) * a) & 0xFF00FF00) >> 8);
}

void Surface::fillRectAlpha(SDL_Rect rect, RGBAColor c) {
	applyClipRect(rect);
	if (rect.w == 0 || rect.h == 0) {
		// Entire rectangle is outside clipping area.
		return;
	}

	if (SDL_MUSTLOCK(raw)) {
		if (SDL_LockSurface(raw) < 0) {
			return;
		}
	}

	SDL_PixelFormat *format = raw->format;
	uint32_t color = c.pixelValue(format);
	uint8_t alpha = c.a;

	uint8_t* edge = static_cast<uint8_t*>(raw->pixels)
	               + rect.y * raw->pitch
	               + rect.x * format->BytesPerPixel;

	// Blending: surf' = surf * (1 - alpha) + fill * alpha

	if (format->BytesPerPixel == 2) {
		uint32_t Rmask = format->Rmask;
		uint32_t Gmask = format->Gmask;
		uint32_t Bmask = format->Bmask;

		// Pre-multiply the fill color. We're hardcoding alpha to 1: 15/16bpp
		// modes are unlikely to have an alpha channel and even if they do,
		// the written alpha isn't used by gmenu2x.
		uint16_t f = (((color & Rmask) * alpha >> 8) & Rmask)
		           | (((color & Gmask) * alpha >> 8) & Gmask)
		           | (((color & Bmask) * alpha >> 8) & Bmask)
		           | format->Amask;
		alpha = 255 - alpha;

		for (auto y = 0; y < rect.h; y++) {
			for (auto x = 0; x < rect.w; x++) {
				uint16_t& pixel = reinterpret_cast<uint16_t*>(edge)[x];
				uint32_t R = ((pixel & Rmask) * alpha >> 8) & Rmask;
				uint32_t G = ((pixel & Gmask) * alpha >> 8) & Gmask;
				uint32_t B = ((pixel & Bmask) * alpha >> 8) & Bmask;
				pixel = uint16_t(R | G | B) + f;
			}
			edge += raw->pitch;
		}
	} else if (format->BytesPerPixel == 4) {
		// Assume the pixel format uses 8 bits per component; we don't care
		// which component is where since they all blend the same.
		uint32_t f = mult8x4(color, alpha); // pre-multiply the fill color
		alpha = 255 - alpha;

		for (auto y = 0; y < rect.h; y++) {
			for (auto x = 0; x < rect.w; x++) {
				uint32_t& pixel = reinterpret_cast<uint32_t*>(edge)[x];
				pixel = mult8x4(pixel, alpha) + f;
			}
			edge += raw->pitch;
		}
	} else {
		assert(false);
	}

	if (SDL_MUSTLOCK(raw)) {
		SDL_UnlockSurface(raw);
	}
}


// OffscreenSurface:

unique_ptr<OffscreenSurface> OffscreenSurface::emptySurface(
		int width, int height)
{
	SDL_Surface *raw = SDL_CreateRGBSurface(
			SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0);
	if (!raw) return unique_ptr<OffscreenSurface>();
	SDL_FillRect(raw, nullptr, SDL_MapRGB(raw->format, 0, 0, 0));
	return unique_ptr<OffscreenSurface>(new OffscreenSurface(raw));
}

unique_ptr<OffscreenSurface> OffscreenSurface::loadImage(
		string const& img, bool loadAlpha)
{
	SDL_Surface *raw = loadPNG(img, loadAlpha);
	if (!raw) {
		DEBUG("Couldn't load surface '%s'\n", img.c_str());
		return unique_ptr<OffscreenSurface>();
	}

	return unique_ptr<OffscreenSurface>(new OffscreenSurface(raw));
}

/**
 * Returns the value resulting from the linear interpolation of the values
 * 'a', considered to be at 0.0, and 'b', considered to be at 1.0, given
 * the specified weight.
 */
static inline uint32_t interpolate(uint32_t a, uint32_t b, float weight)
{
	return static_cast<uint32_t>(a * (1.0f - weight) + b * weight);
}

unique_ptr<OffscreenSurface> OffscreenSurface::scaleTo(int newWidth, int newHeight)
{
	if (newWidth == raw->w && newHeight == raw->h) {
		return unique_ptr<OffscreenSurface>(new OffscreenSurface(*this));
	}

	SDL_PixelFormat *format = raw->format;
	uint32_t const& rMask = format->Rmask;
	uint32_t const& gMask = format->Gmask;
	uint32_t const& bMask = format->Bmask;
	uint32_t const& aMask = format->Amask;

	SDL_Surface *result = SDL_CreateRGBSurface(SDL_SWSURFACE,
		newWidth, newHeight, format->BitsPerPixel,
		rMask, gMask, bMask, aMask
	);

	if (!result) {
		return unique_ptr<OffscreenSurface>();
	}
	if (SDL_MUSTLOCK(result)) {
		if (SDL_LockSurface(result) < 0) {
			return unique_ptr<OffscreenSurface>();
		}
	}
	if (SDL_MUSTLOCK(raw)) {
		if (SDL_LockSurface(raw) < 0) {
			return unique_ptr<OffscreenSurface>();
		}
	}

	uint8_t *oLine = static_cast<uint8_t*>(result->pixels);

	/* In the algorithm below, only 4 neighboring pixels in the input image
	 * are ever considered for any given output pixel, even if scaling the
	 * input image to below half of either of its dimensions. This allows
	 * the algorithm's run time to be determined by the area of the output
	 * image instead of the input, but the output image may be uglier.
	 * o* variables refer to the output image; i* refer to the input image. */
	for (int oY = 0; oY < newHeight; oY++) {
		float iY = static_cast<float>(oY * (raw->h - 1)) / newHeight;
		int iY_i = (int) iY; // Index of the top pixel in the input image
		float iY_f = iY - iY_i; // Interpolant between input rows
		uint8_t *iLine_top = static_cast<uint8_t*>(raw->pixels)
				+ iY_i * raw->pitch;
		uint8_t *iLine_bottom = static_cast<uint8_t*>(raw->pixels)
				+ (iY_i + 1) * raw->pitch;

		for (int oX = 0; oX < newWidth; oX++) {
			float iX = static_cast<float>(oX * (raw->w - 1)) / newWidth;
			int iX_i = (int) iX; // Index of the left pixel in the input row
			float iX_f = iX - iX_i; // Interpolant between input pixels

			/* Split the color components of each pixel, interpolate them
			 * where they are, mask off the lower bits that result from the
			 * interpolation, then write the result once the value of all
			 * color components is known. For example, if Red is at bits
			 * 31..24, it is interpolated in bits 31..24, but bits 23..0
			 * may get some residual values, so those are masked off. */
			if (format->BytesPerPixel == 2) {
				uint16_t& iA = reinterpret_cast<uint16_t*>(iLine_top)[iX_i];
				uint16_t& iB = reinterpret_cast<uint16_t*>(iLine_top)[iX_i + 1];
				uint16_t& iC = reinterpret_cast<uint16_t*>(iLine_bottom)[iX_i];
				uint16_t& iD = reinterpret_cast<uint16_t*>(iLine_bottom)[iX_i + 1];
				uint32_t rA = iA & rMask, gA = iA & gMask, bA = iA & bMask;
				uint32_t rB = iB & rMask, gB = iB & gMask, bB = iB & bMask;
				uint32_t rC = iC & rMask, gC = iC & gMask, bC = iC & bMask;
				uint32_t rD = iD & rMask, gD = iD & gMask, bD = iD & bMask;
				reinterpret_cast<uint16_t*>(oLine)[oX] =
					(interpolate(
						interpolate(rA, rB, iX_f), interpolate(rC, rD, iX_f), iY_f
					) & rMask) |
					(interpolate(
						interpolate(gA, gB, iX_f), interpolate(gC, gD, iX_f), iY_f
					) & gMask) |
					(interpolate(
						interpolate(bA, bB, iX_f), interpolate(bC, bD, iX_f), iY_f
					) & bMask);
			} else if (format->BytesPerPixel == 4) {
				uint32_t& iA = reinterpret_cast<uint32_t*>(iLine_top)[iX_i];
				uint32_t& iB = reinterpret_cast<uint32_t*>(iLine_top)[iX_i + 1];
				uint32_t& iC = reinterpret_cast<uint32_t*>(iLine_bottom)[iX_i];
				uint32_t& iD = reinterpret_cast<uint32_t*>(iLine_bottom)[iX_i + 1];
				uint32_t rA = iA & rMask, gA = iA & gMask, bA = iA & bMask, aA = iA & aMask;
				uint32_t rB = iB & rMask, gB = iB & gMask, bB = iB & bMask, aB = iB & aMask;
				uint32_t rC = iC & rMask, gC = iC & gMask, bC = iC & bMask, aC = iC & aMask;
				uint32_t rD = iD & rMask, gD = iD & gMask, bD = iD & bMask, aD = iD & aMask;
				reinterpret_cast<uint32_t*>(oLine)[oX] =
					(interpolate(
						interpolate(rA, rB, iX_f), interpolate(rC, rD, iX_f), iY_f
					) & rMask) |
					(interpolate(
						interpolate(gA, gB, iX_f), interpolate(gC, gD, iX_f), iY_f
					) & gMask) |
					(interpolate(
						interpolate(bA, bB, iX_f), interpolate(bC, bD, iX_f), iY_f
					) & bMask) |
					(interpolate(
						interpolate(aA, aB, iX_f), interpolate(aC, aD, iX_f), iY_f
					) & aMask);
			} else {
				assert(false);
			}
		}

		oLine += result->pitch;
	}

	if (SDL_MUSTLOCK(raw)) {
		SDL_UnlockSurface(raw);
	}
	if (SDL_MUSTLOCK(result)) {
		SDL_UnlockSurface(result);
	}

	return unique_ptr<OffscreenSurface>(new OffscreenSurface(result));
}

unique_ptr<OffscreenSurface> OffscreenSurface::scaleToFit(int newWidth, int newHeight)
{
	float widthRatio = (float) newWidth / raw->w,
	      heightRatio = (float) newHeight / raw->h;
	return (widthRatio < heightRatio) ? scaleTo(newWidth, (int) (raw->h * widthRatio))
	     : (heightRatio < widthRatio) ? scaleTo((int) (raw->w * heightRatio), newHeight)
	     : scaleTo(newWidth, newHeight);
}

OffscreenSurface::OffscreenSurface(OffscreenSurface&& other)
	: Surface(other.raw)
{
	other.raw = nullptr;
}

OffscreenSurface::~OffscreenSurface()
{
	SDL_FreeSurface(raw);
}

OffscreenSurface& OffscreenSurface::operator=(OffscreenSurface other)
{
	swap(other);
	return *this;
}

void OffscreenSurface::swap(OffscreenSurface& other)
{
	std::swap(raw, other.raw);
}

void OffscreenSurface::convertToDisplayFormat() {
	SDL_Surface *newSurface = SDL_DisplayFormat(raw);
	if (newSurface) {
		SDL_FreeSurface(raw);
		raw = newSurface;
	}
}


// OutputSurface:

unique_ptr<OutputSurface> OutputSurface::open(
		int width, int height, int bitsPerPixel)
{
	SDL_ShowCursor(SDL_DISABLE);
	SDL_Surface *raw = SDL_SetVideoMode(
		width, height, bitsPerPixel, SDL_HWSURFACE | SDL_DOUBLEBUF);
	return unique_ptr<OutputSurface>(raw ? new OutputSurface(raw) : nullptr);
}

void OutputSurface::flip() {
	SDL_Flip(raw);
}
