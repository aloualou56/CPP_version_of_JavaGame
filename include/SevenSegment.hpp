#ifndef SevenSegment_hpp
#define SevenSegment_hpp

#include "SDL3/SDL.h"

// Minimal seven-segment digit renderer (0-9) used for on-screen numeral
// hints (character-select "press 1 / 2"). The project has no text/font
// library linked (only SDL3 + SDL3_image), so this avoids adding a new
// dependency just to draw two digits, and is far less error-prone than a
// hand-typed bitmap glyph table for a full alphabet.
namespace SevenSegment {
    // Draws `digit` (0-9) as a classic seven-segment numeral at (x, y) with
    // the given cell size, using filled rectangles.
    void drawDigit(SDL_Renderer* renderer, int digit, float x, float y, float width, float height, SDL_Color color);
}

#endif
