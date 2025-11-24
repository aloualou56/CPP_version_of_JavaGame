# Collision Fix - Visual Guide

## Before the Fix

```
Bush Sprite with Semi-Transparent Glow:
┌─────────────────────────────────┐
│         (invisible)              │ ← Alpha 50-100 (very transparent glow)
│    ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒         │ ← Alpha 150 (semi-transparent edge)
│   ▒████████████████████▒        │ ← Alpha 220 (nearly opaque bush)
│  ▒██████████████████████▒       │
│  ▒██████████████████████▒       │ ← Visible bush
│  ▒██████████████████████▒       │
│   ▒████████████████████▒        │
│    ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒         │
│         (invisible)              │
└─────────────────────────────────┘

OLD Collision Detection (Alpha > 128):
┌─────────────────────────────────┐
│         XXXXXXXXXX               │ ← COLLISION DETECTED HERE!
│    XXXXXXXXXXXXXXXXXXXX          │ ← Semi-transparent pixels cause collision
│   XXXXXXXXXXXXXXXXXXXXXXX        │
│  XXXXXXXXXXXXXXXXXXXXXXXXX       │
│  XXXXXXXXXXXXXXXXXXXXXXXXX       │ ← Player hits invisible wall here
│  XXXXXXXXXXXXXXXXXXXXXXXXX       │    before touching visible sprite
│   XXXXXXXXXXXXXXXXXXXXXXX        │
│    XXXXXXXXXXXXXXXXXXXX          │
│         XXXXXXXXXX               │
└─────────────────────────────────┘
       ↑
   Invisible Wall!
   Player blocked too early


## After the Fix

NEW Collision Detection (Alpha > 200):
┌─────────────────────────────────┐
│         (no collision)           │ ← No collision in glow area
│    (no collision)                │ ← Semi-transparent edge ignored
│   ████████████████████           │ ← Collision only on solid bush
│  ██████████████████████          │
│  ██████████████████████          │ ← Player can get close to sprite
│  ██████████████████████          │    before collision
│   ████████████████████           │
│    (no collision)                │
│         (no collision)           │
└─────────────────────────────────┘
       ↑
   Natural collision!
   Player can approach visual sprite


## Alpha Threshold Comparison

Pixel Alpha Values:
0  ─────────────────────────── Fully Transparent
50  ← Glow/Shadow effects
100 
128 ← OLD threshold (too sensitive)
150 ← Anti-aliasing edges
200 ← NEW threshold (just right!)
220 
255 ─────────────────────────── Fully Opaque

Effect of Change:
- Alpha 0-128:   Always transparent (no collision)     [UNCHANGED]
- Alpha 129-200: NOW transparent (no collision)        [CHANGED - This fixes the bug!]
- Alpha 201-255: Solid (collision detected)            [UNCHANGED]
```

## Game Behavior

### Before Fix (Bushes with invisible wall):
```
Player Movement:

 🚶              🌳
  →  →  →  ⚠️ BLOCKED!
           ↑
      Invisible wall
  Can't reach the bush
```

### After Fix (Bushes work correctly):
```
Player Movement:

 🚶              🌳
  →  →  →  →  😊 Touch!
                ↑
           Natural collision
  Can reach the visible sprite
```

## Technical Implementation

### Code Changes in ImageMask.hpp:

**Line 42 - Bounding Box Detection:**
```cpp
// OLD:
if (a > 128) {
    // Marks pixel as part of collision mask
}

// NEW:
if (a > 200) {  // Use higher threshold to ignore semi-transparent pixels
    // Only nearly-opaque pixels are part of collision mask
}
```

**Line 74 - Mask Creation:**
```cpp
// OLD:
mask[y][x] = (a > 128);

// NEW:
mask[y][x] = (a > 200);  // Same threshold as bounding box
```

### Why This Works:

1. **Sprite artists** often add soft edges, glows, or shadows using semi-transparent pixels
2. **Old threshold (128)** treated anything more than 50% transparent as solid
3. **New threshold (200)** requires pixels to be nearly opaque (78%+ opacity) to be solid
4. **Result**: Visual effects don't interfere with collision, only the main sprite does

## Testing Instructions

1. Launch Game.exe
2. Press TAB to enable debug mode
3. Walk towards a bush (thamnos_tonia) - green sprite
4. Observe that:
   - Red box = Full collision rectangle
   - Green box = Pixel-perfect mask (should be tight around visible sprite)
   - You can now walk close to the bush without hitting an invisible wall

5. Compare with a rock:
   - Rocks should work the same as before (they already had tight collision)
   - Both rocks and bushes should now have consistent collision behavior
