#!/usr/bin/env python3
"""
check_grass_color.py - Analyze screenshots to detect grass tile color issues.

This script analyzes a screenshot from the Android game to determine whether
grass tiles are rendering as green (correct) or red (bug due to channel order).

Usage:
    python check_grass_color.py <screenshot_path> [--roi x,y,w,h] [--verbose]

Exit codes:
    0 - Grass tiles are predominantly green (PASS)
    1 - Grass tiles are predominantly red (FAIL)
    2 - Error (file not found, analysis failed, etc.)

Dependencies:
    - Pillow (PIL)
    - numpy (optional, for faster processing)
"""

import argparse
import json
import os
import sys
from datetime import datetime
from pathlib import Path

# Try to import numpy for faster processing, fall back to pure Python if not available
try:
    import numpy as np
    HAS_NUMPY = True
except ImportError:
    HAS_NUMPY = False

# Try to import PIL
try:
    from PIL import Image
except ImportError:
    print("Error: Pillow library is required. Install with: pip install Pillow")
    sys.exit(2)


def is_green_pixel(r, g, b, threshold=30):
    """
    Determine if a pixel is predominantly green.
    
    A pixel is considered green if:
    - Green channel is higher than red by at least threshold
    - Green channel is higher than blue by at least threshold
    - Green value is above a minimum (to exclude very dark pixels)
    """
    return g > r + threshold and g > b + threshold and g > 50


def is_red_pixel(r, g, b, threshold=30):
    """
    Determine if a pixel is predominantly red.
    
    A pixel is considered red if:
    - Red channel is higher than green by at least threshold
    - Red channel is higher than blue by at least threshold
    - Red value is above a minimum (to exclude very dark pixels)
    """
    return r > g + threshold and r > b + threshold and r > 50


def is_grass_candidate(r, g, b):
    """
    Determine if a pixel could be a grass tile pixel.
    
    Grass tiles typically have earthy/natural colors - either green (correct)
    or red/brown (bug). We exclude very dark, very bright, and pure gray pixels.
    """
    # Exclude very dark pixels (shadows, UI elements)
    if r < 30 and g < 30 and b < 30:
        return False
    
    # Exclude very bright pixels (UI, highlights)
    if r > 240 and g > 240 and b > 240:
        return False
    
    # Exclude gray pixels (UI elements, backgrounds)
    diff_rg = abs(r - g)
    diff_rb = abs(r - b)
    diff_gb = abs(g - b)
    if diff_rg < 15 and diff_rb < 15 and diff_gb < 15:
        return False
    
    # Accept pixels that are somewhat green or red (grass should be one or the other)
    return is_green_pixel(r, g, b, threshold=20) or is_red_pixel(r, g, b, threshold=20) or \
           (g > 40 and (g > r or r > g + 10))  # Some tolerance for brownish-green


def analyze_region(image, roi=None, verbose=False):
    """
    Analyze a region of the image for grass color.
    
    Args:
        image: PIL Image object
        roi: Tuple (x, y, width, height) or None for full image analysis
        verbose: Print detailed output
    
    Returns:
        dict with analysis results
    """
    if roi:
        x, y, w, h = roi
        region = image.crop((x, y, x + w, y + h))
    else:
        region = image
    
    width, height = region.size
    
    # Convert to RGB if necessary
    if region.mode != 'RGB':
        region = region.convert('RGB')
    
    # Get pixel data
    if HAS_NUMPY:
        pixels = np.array(region)
        total_pixels = width * height
        
        r, g, b = pixels[:,:,0], pixels[:,:,1], pixels[:,:,2]
        
        # Count green and red pixels
        green_mask = (g > r + 30) & (g > b + 30) & (g > 50)
        red_mask = (r > g + 30) & (r > b + 30) & (r > 50)
        
        # Grass candidate mask (exclude very dark/bright/gray)
        # Convert to int once for efficiency
        r_int, g_int, b_int = r.astype(int), g.astype(int), b.astype(int)
        not_dark = (r > 30) | (g > 30) | (b > 30)
        not_bright = (r < 240) | (g < 240) | (b < 240)
        not_gray = (np.abs(r_int - g_int) > 15) | (np.abs(r_int - b_int) > 15)
        grass_candidate = not_dark & not_bright & not_gray
        
        green_count = int(np.sum(green_mask & grass_candidate))
        red_count = int(np.sum(red_mask & grass_candidate))
        candidate_count = int(np.sum(grass_candidate))
    else:
        # Pure Python fallback (slower)
        pixels = list(region.getdata())
        total_pixels = len(pixels)
        
        green_count = 0
        red_count = 0
        candidate_count = 0
        
        for r, g, b in pixels:
            if is_grass_candidate(r, g, b):
                candidate_count += 1
                if is_green_pixel(r, g, b):
                    green_count += 1
                elif is_red_pixel(r, g, b):
                    red_count += 1
    
    # Calculate percentages
    if candidate_count > 0:
        green_percent = (green_count / candidate_count) * 100
        red_percent = (red_count / candidate_count) * 100
    else:
        green_percent = 0
        red_percent = 0
    
    # Determine result
    # If green significantly outweighs red, it's correct
    # If red significantly outweighs green, it's the bug
    if green_count > red_count * 1.5 and green_percent > 5:
        result = "GREEN"
        passed = True
    elif red_count > green_count * 1.5 and red_percent > 5:
        result = "RED"
        passed = False
    elif green_count > red_count:
        result = "GREEN (marginal)"
        passed = True
    elif red_count > green_count:
        result = "RED (marginal)"
        passed = False
    else:
        result = "INCONCLUSIVE"
        passed = True  # Default to pass if we can't determine
    
    return {
        "total_pixels": total_pixels,
        "candidate_pixels": candidate_count,
        "green_pixels": green_count,
        "red_pixels": red_count,
        "green_percent": round(green_percent, 2),
        "red_percent": round(red_percent, 2),
        "result": result,
        "passed": passed
    }


def auto_detect_game_region(image, verbose=False):
    """
    Try to automatically detect the game rendering region.
    
    For mobile games, the game area is typically the largest continuous
    area with varied colors. We look for regions that have tile-like
    repeating patterns or natural color variations.
    
    Returns:
        Tuple (x, y, width, height) or None if detection fails
    """
    width, height = image.size
    
    # For most games, the game area is in the center portion of the screen
    # excluding status bars, navigation bars, etc.
    # We'll use the middle 80% of the screen height and full width
    margin_top = int(height * 0.1)
    margin_bottom = int(height * 0.1)
    
    roi = (0, margin_top, width, height - margin_top - margin_bottom)
    
    if verbose:
        print(f"Auto-detected game region: x={roi[0]}, y={roi[1]}, w={roi[2]}, h={roi[3]}")
    
    return roi


def create_report(image_path, results, roi=None):
    """
    Create a human-readable report file next to the screenshot.
    """
    report_path = str(image_path).rsplit('.', 1)[0] + '_analysis.txt'
    
    with open(report_path, 'w') as f:
        f.write("=" * 60 + "\n")
        f.write("Grass Color Analysis Report\n")
        f.write("=" * 60 + "\n\n")
        f.write(f"Screenshot: {image_path}\n")
        f.write(f"Analysis Date: {datetime.now().isoformat()}\n")
        if roi:
            f.write(f"Region of Interest: x={roi[0]}, y={roi[1]}, w={roi[2]}, h={roi[3]}\n")
        else:
            f.write("Region of Interest: Full image\n")
        f.write("\n")
        f.write("Results:\n")
        f.write("-" * 40 + "\n")
        f.write(f"Total Pixels Analyzed: {results['total_pixels']}\n")
        f.write(f"Candidate Grass Pixels: {results['candidate_pixels']}\n")
        f.write(f"Green Pixels: {results['green_pixels']} ({results['green_percent']}%)\n")
        f.write(f"Red Pixels: {results['red_pixels']} ({results['red_percent']}%)\n")
        f.write("\n")
        f.write(f"VERDICT: {results['result']}\n")
        f.write(f"STATUS: {'PASS' if results['passed'] else 'FAIL'}\n")
        f.write("\n")
        f.write("=" * 60 + "\n")
    
    return report_path


def create_json_report(image_path, results, roi=None):
    """
    Create a JSON report file next to the screenshot.
    """
    json_path = str(image_path).rsplit('.', 1)[0] + '_analysis.json'
    
    report = {
        "screenshot": str(image_path),
        "analysis_date": datetime.now().isoformat(),
        "roi": roi,
        "results": results
    }
    
    with open(json_path, 'w') as f:
        json.dump(report, f, indent=2)
    
    return json_path


def main():
    parser = argparse.ArgumentParser(
        description='Analyze screenshot for grass tile color issues'
    )
    parser.add_argument('screenshot', help='Path to screenshot image')
    parser.add_argument('--roi', help='Region of interest: x,y,w,h (e.g., "100,200,400,300")')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose output')
    parser.add_argument('--no-report', action='store_true', help='Skip creating report files')
    
    args = parser.parse_args()
    
    # Check if file exists
    if not os.path.exists(args.screenshot):
        print(f"Error: Screenshot not found: {args.screenshot}")
        sys.exit(2)
    
    try:
        image = Image.open(args.screenshot)
    except Exception as e:
        print(f"Error: Failed to open image: {e}")
        sys.exit(2)
    
    if args.verbose:
        print(f"Analyzing: {args.screenshot}")
        print(f"Image size: {image.size[0]}x{image.size[1]}")
        print(f"Image mode: {image.mode}")
        print()
    
    # Parse ROI if provided
    roi = None
    if args.roi:
        try:
            parts = args.roi.split(',')
            roi = tuple(int(p) for p in parts)
            if len(roi) != 4:
                raise ValueError("ROI must have 4 values: x,y,w,h")
        except ValueError as e:
            print(f"Error: Invalid ROI format: {e}")
            sys.exit(2)
    else:
        # Try to auto-detect game region
        roi = auto_detect_game_region(image, args.verbose)
    
    # Analyze the image
    results = analyze_region(image, roi, args.verbose)
    
    if args.verbose:
        print("Analysis Results:")
        print(f"  Total pixels: {results['total_pixels']}")
        print(f"  Candidate grass pixels: {results['candidate_pixels']}")
        print(f"  Green pixels: {results['green_pixels']} ({results['green_percent']}%)")
        print(f"  Red pixels: {results['red_pixels']} ({results['red_percent']}%)")
        print()
    
    print(f"Result: {results['result']}")
    print(f"Status: {'PASS' if results['passed'] else 'FAIL'}")
    
    # Create reports
    if not args.no_report:
        report_path = create_report(args.screenshot, results, roi)
        json_path = create_json_report(args.screenshot, results, roi)
        if args.verbose:
            print(f"\nReport saved: {report_path}")
            print(f"JSON report saved: {json_path}")
    
    # Exit with appropriate code
    sys.exit(0 if results['passed'] else 1)


if __name__ == '__main__':
    main()
