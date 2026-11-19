#!/usr/bin/env python3
# =============================================================================
# tools/capture_screenshots.py
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D) <electrohobby3d@gmail.com>
# GPL-3.0-or-later - see LICENSE
# =============================================================================
"""Retakes the screenshots of the README, headless and with made-up sample data (nothing real is behind them, and no window opens on the desktop).

    py -3 tools/capture_screenshots.py              # writes images/screenshots/<file> for every screen listed in docs/screenshots.json
    py -3 tools/capture_screenshots.py --out DIR    # writes them in DIR instead, to look at them first

It runs tools/make_screenshots.mjs (headless Microsoft Edge through Playwright; see the header of that file for what it needs), then keeps only the screens of
docs/screenshots.json and shrinks every picture that weighs more than the limit (at most 400 KB each) by reducing its palette. Pillow is needed for that last step.

The pictures of the wall screen (wall-disarmed.png, wall-alarm.png) are not taken by the browser: the screen simulator draws them (simulator/, then
simulator/ppm_to_png.py). Pass the folder that holds its PNG files (the language folders, as ppm_to_png.py writes them) with --simulator-shots DIR to copy them too;
without it they are skipped and said so.
"""
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
MAX_BYTES = 400 * 1024
TARGET_BYTES = 380 * 1024
# the pictures that come from the screen simulator: file of the README -> (language folder, picture) of simulator/ppm_to_png.py
SIMULATOR_SCREENS = {"wall-disarmed.png": ("en", "disarmed.png"), "wall-alarm.png": ("en", "alarm.png")}


def shrink(source: Path, target: Path) -> int:
    """Copies source to target; when it is too heavy, reduces it to a 256-colour palette (then fewer colours) until it fits. Returns the final size."""
    if source.stat().st_size <= TARGET_BYTES:
        shutil.copyfile(source, target)
        return target.stat().st_size
    try:
        from PIL import Image
    except ImportError as error:  # pragma: no cover - depends on the machine
        raise SystemExit(f"{source.name} weighs {source.stat().st_size} bytes and Pillow is not installed to shrink it: py -3 -m pip install pillow") from error
    image = Image.open(source).convert("RGB")
    for colours in (256, 192, 128, 96, 64):
        image.quantize(colors=colours, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE).save(target, optimize=True)
        if target.stat().st_size <= TARGET_BYTES:
            break
    return target.stat().st_size


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", type=Path, default=REPO / "images" / "screenshots")
    parser.add_argument("--simulator-shots", type=Path, default=None, help="folder with the PNG files of the screen simulator (<language>/<scene>.png), for the wall screen")
    args = parser.parse_args(argv)
    screens = json.loads((REPO / "docs" / "screenshots.json").read_text(encoding="utf-8"))["screens"]
    args.out.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="armor-shots-") as raw:
        subprocess.run(["node", str(REPO / "tools" / "make_screenshots.mjs"), "--out", raw], cwd=REPO, check=True)
        for screen in screens:
            name = screen["file"]
            if name in SIMULATOR_SCREENS:
                if args.simulator_shots is None:
                    print(f"skipped: {name} is drawn by the screen simulator (pass --simulator-shots DIR)")
                    continue
                language, scene = SIMULATOR_SCREENS[name]
                source = args.simulator_shots / language / scene
            else:
                source = Path(raw) / name
            if not source.is_file():
                print(f"missing: {name}")
                return 1
            size = shrink(source, args.out / name)
            print(f"{name}: {size} bytes")
            if size > MAX_BYTES:
                print(f"{name} is still heavier than {MAX_BYTES} bytes")
                return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
