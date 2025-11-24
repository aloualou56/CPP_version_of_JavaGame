# Collision Bug Fix Documentation

## Problem Description

The game had a collision detection bug where:
- **Rocks**: Collision worked correctly - you could get close to the visual sprite before collision occurred
- **Bushes (thamnos_tonia)**: There was an "invisible wall" - collision occurred before touching the visible sprite

## Root Cause

The bug was caused by the alpha threshold used in pixel-perfect collision detection. The code in `src/ImageMask.hpp` was treating any pixel with alpha > 128 as solid for collision purposes.

### Technical Explanation

When sprite images are created with anti-aliasing, shadows, or glow effects, they often have semi-transparent pixels around the edges. These pixels have alpha values between 129 and 254 (not fully opaque, but not fully transparent either).

With the original threshold of 128:
- Pixels with alpha 0-128: Treated as transparent (no collision)
- Pixels with alpha 129-255: Treated as solid (collision occurs)

This meant that sprites with visual effects like soft edges or glows would have collision detection that extended beyond the visible sprite, creating an "invisible wall" effect.

## Solution

The fix involved two changes:

### 1. Increased Alpha Threshold (src/ImageMask.hpp)

Changed the alpha threshold from **128 to 200**:

```cpp
// OLD CODE (line 42):
if (a > 128) {
    // pixel considered solid
}

// NEW CODE:
if (a > 200) {  // Use higher threshold to ignore semi-transparent pixels
    // pixel considered solid
}
```

This change was applied in two places:
1. When finding the bounding box of solid pixels (line 42)
2. When creating the pixel mask array (line 74)

**Effect**: Only nearly-opaque pixels (alpha > 200) are now considered solid. Semi-transparent pixels used for anti-aliasing and visual effects (alpha 129-200) are now ignored for collision purposes.

### 2. Added Fallback Mask Loading (src/ECS/ColliderComponent.hpp)

Added a fallback case for bushes in the mask loading code:

```cpp
// Added to line 58-59:
} else if (tag == "bush") {
   mask.loadFromPNG("sprites/objects/thamnos_tonia.png");
```

**Effect**: Ensures that even if the mask fails to load from the SpriteComponent for some reason, bushes will still get their collision mask loaded from the sprite file.

## Files Modified

1. **src/ImageMask.hpp**
   - Line 42: Changed alpha threshold from 128 to 200 (with Greek comment)
   - Line 74: Changed alpha threshold from 128 to 200 (with Greek comment)

2. **src/ECS/ColliderComponent.hpp**
   - Lines 58-59: Added fallback mask loading for "bush" tag

3. **src/ECS/PositionComponent.cpp**
   - Line 55: Added clarifying Greek comment about coordinate conversion

## Testing

To verify the fix works:

1. Run the game with debug mode enabled (press TAB key)
2. Approach a rock - you should see the red collision box and green mask box align closely with the visible sprite
3. Approach a bush - the collision should now align with the visible sprite instead of having an invisible wall
4. You should be able to walk close to bushes without hitting an invisible barrier

## Technical Background

### Pixel-Perfect Collision Detection

The game uses a pixel-perfect collision system that:
1. Loads the sprite image and analyzes each pixel's alpha channel
2. Creates a binary mask of "solid" vs "transparent" pixels
3. Crops the mask to the minimal bounding box of solid pixels
4. During collision detection, checks if solid pixels overlap between two sprites

### Alpha Channel Values

- Alpha 0: Fully transparent (invisible)
- Alpha 1-127: Very transparent to semi-transparent
- Alpha 128-200: Semi-transparent (visible but somewhat transparent)
- Alpha 201-255: Nearly opaque to fully opaque

### Why The Threshold Matters

Different sprite creation tools and artists use different techniques:
- Simple sprites might only use alpha 0 (transparent) and 255 (opaque)
- Anti-aliased sprites use gradual alpha transitions at edges (e.g., 180, 200, 220, 240, 255)
- Sprites with glows or shadows have larger semi-transparent areas

The threshold determines which pixels should cause collision. Too low (like 128) creates invisible walls around soft-edged sprites. Too high (like 254) might allow players to walk through visible parts of sprites.

The value of 200 provides a good balance:
- Fully opaque pixels (alpha 201-255) cause collision ✓
- Anti-aliasing edges (alpha 150-200) don't cause collision ✓
- Very transparent effects (alpha 1-150) don't cause collision ✓

## Future Improvements

Potential enhancements for the collision system:

1. **Per-sprite threshold**: Allow different sprites to specify their own alpha thresholds
2. **Configuration file**: Make the threshold configurable without recompiling
3. **Visual editor**: Create a tool to visualize and adjust collision masks
4. **Performance optimization**: Cache the pixel-perfect checks or use spatial hashing

## Credits

- Bug reported by: aloualou56
- Fix implemented by: GitHub Copilot
- Date: November 24, 2025
