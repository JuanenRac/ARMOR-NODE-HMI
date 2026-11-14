#!/usr/bin/env python3
# =============================================================================
# ARMOR-NODE-HMI - simulator/ppm_to_png.py
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D)
# GPL-3.0-or-later - see LICENSE
# =============================================================================
"""Turns the PPM pictures of the screen simulator into 128-colour PNG files (at most 400 KiB each, the limit of the pictures of the README).

    python simulator/ppm_to_png.py SHOTS_DIR OUT_DIR      # OUT_DIR/<language>/<scene>.png

Needs Pillow. Exit code 0 only when every picture was written and fits.
"""
import sys
from pathlib import Path

from PIL import Image

LIMIT = 400 * 1024


def convert(shots: Path, out: Path) -> int:
    failures = 0
    files = sorted(shots.glob("*.ppm"))
    if not files:
        print("no .ppm pictures in", shots)
        return 1
    for ppm in files:
        language, _, scene = ppm.stem.partition("_")
        target = out / language / f"{scene}.png"
        target.parent.mkdir(parents=True, exist_ok=True)
        with Image.open(ppm) as image:
            image.convert("RGB").quantize(colors=128, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE).save(target, optimize=True)
        size = target.stat().st_size
        if size > LIMIT:
            print(f"{target}: {size} bytes is over the limit of {LIMIT}")
            failures += 1
    print(f"PNG={'FAIL' if failures else 'PASS'} pictures={len(files)}")
    return 1 if failures else 0


def main(argv: list[str]) -> int:
    if len(argv) != 3 or argv[1] in ("-h", "--help"):
        print(__doc__)
        return 0 if len(argv) == 2 else 64
    return convert(Path(argv[1]), Path(argv[2]))


if __name__ == "__main__":
    sys.exit(main(sys.argv))
