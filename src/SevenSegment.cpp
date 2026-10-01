#include "SevenSegment.hpp"

namespace SevenSegment {

// Segment bits: a=top, b=top-right, c=bottom-right, d=bottom, e=bottom-left,
// f=top-left, g=middle. Standard seven-segment digit patterns.
static const bool kSegments[10][7] = {
    /*0*/ { true,  true,  true,  true,  true,  true,  false },
    /*1*/ { false, true,  true,  false, false, false, false },
    /*2*/ { true,  true,  false, true,  true,  false, true  },
    /*3*/ { true,  true,  true,  true,  false, false, true  },
    /*4*/ { false, true,  true,  false, false, true,  true  },
    /*5*/ { true,  false, true,  true,  false, true,  true  },
    /*6*/ { true,  false, true,  true,  true,  true,  true  },
    /*7*/ { true,  true,  true,  false, false, false, false },
    /*8*/ { true,  true,  true,  true,  true,  true,  true  },
    /*9*/ { true,  true,  true,  true,  false, true,  true  },
};

void drawDigit(SDL_Renderer* renderer, int digit, float x, float y, float width, float height, SDL_Color color) {
    if (!renderer || digit < 0 || digit > 9) return;

    float thickness = width * 0.22f;
    const bool* seg = kSegments[digit];

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // a: top
    if (seg[0]) { SDL_FRect r{ x + thickness, y, width - 2 * thickness, thickness }; SDL_RenderFillRect(renderer, &r); }
    // b: top-right
    if (seg[1]) { SDL_FRect r{ x + width - thickness, y, thickness, height / 2.0f }; SDL_RenderFillRect(renderer, &r); }
    // c: bottom-right
    if (seg[2]) { SDL_FRect r{ x + width - thickness, y + height / 2.0f, thickness, height / 2.0f }; SDL_RenderFillRect(renderer, &r); }
    // d: bottom
    if (seg[3]) { SDL_FRect r{ x + thickness, y + height - thickness, width - 2 * thickness, thickness }; SDL_RenderFillRect(renderer, &r); }
    // e: bottom-left
    if (seg[4]) { SDL_FRect r{ x, y + height / 2.0f, thickness, height / 2.0f }; SDL_RenderFillRect(renderer, &r); }
    // f: top-left
    if (seg[5]) { SDL_FRect r{ x, y, thickness, height / 2.0f }; SDL_RenderFillRect(renderer, &r); }
    // g: middle
    if (seg[6]) { SDL_FRect r{ x + thickness, y + height / 2.0f - thickness / 2.0f, width - 2 * thickness, thickness }; SDL_RenderFillRect(renderer, &r); }
}

}
