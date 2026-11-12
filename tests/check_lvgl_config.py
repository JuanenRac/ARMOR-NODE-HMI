#!/usr/bin/env python3
"""Check the LVGL memory configuration of the panel without a build.

Usage:  python tests/check_lvgl_config.py

LVGL 9 has no `LV_MEM_CUSTOM` (that is a symbol of LVGL 8); the pool is moved to the C allocator with `LV_USE_CLIB_MALLOC`, so the large blocks go to the external RAM instead of a
fixed pool inside the internal RAM. A configuration line for a symbol that does not exist is silently dropped by the build, so this checks the text: the symbol of LVGL 9 is on, the
symbols of LVGL 8 are not set in any sdkconfig file, and the version that gets locked is LVGL 9.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LVGL8_ONLY = ("CONFIG_LV_MEM_CUSTOM", "CONFIG_LV_MEM_SIZE", "CONFIG_LV_MEM_ADR", "CONFIG_LV_MEM_POOL_INCLUDE", "CONFIG_LV_MEM_POOL_ALLOC")


def active_lines(path: Path) -> list[str]:
    return [line.strip() for line in path.read_text(encoding="utf-8").splitlines() if line.strip() and not line.lstrip().startswith("#")]


def main() -> int:
    failures: list[str] = []
    files = sorted(ROOT.glob("sdkconfig.defaults*")) + sorted(ROOT.glob("sdkconfig.board.*"))
    if not files:
        failures.append("no sdkconfig defaults found")
    defaults = ROOT / "sdkconfig.defaults"
    if "CONFIG_LV_USE_CLIB_MALLOC=y" not in active_lines(defaults):
        failures.append("sdkconfig.defaults does not set CONFIG_LV_USE_CLIB_MALLOC=y")
    for path in files:
        for line in active_lines(path):
            if line.split("=", 1)[0] in LVGL8_ONLY:
                failures.append(f"{path.name}: {line} is a symbol of LVGL 8")
    lock = ROOT / "dependencies.lock"
    if lock.is_file():
        match = re.search(r"lvgl/lvgl:\s*\n(?:\s+.*\n)*?\s+version:\s*([0-9]+)\.", lock.read_text(encoding="utf-8"))
        if match and match.group(1) != "9":
            failures.append(f"dependencies.lock locks LVGL {match.group(1)}, the configuration is for LVGL 9")
    manifest = (ROOT / "main" / "idf_component.yml").read_text(encoding="utf-8")
    if not re.search(r'lvgl/lvgl:\s*"[^"]*9\.', manifest):
        failures.append("main/idf_component.yml does not ask for LVGL 9")
    # The loops that run for ever read the few fields they need under the lock (with_settings) and never copy the whole settings (about forty strings): that is memory churn in the internal RAM.
    app = (ROOT / "main" / "app_main.cpp").read_text(encoding="utf-8")
    body = re.search(r"void presence_task\(void\*\) \{.*?\n\}\n", app, re.S)
    if body is None or "store::settings()" in body.group(0):
        failures.append("presence_task copies the whole settings on every heartbeat")
    screen_loop = re.search(r"if \(now_ms - read_at_ms >= 3000\) \{.*?\n      \}", app, re.S)
    if screen_loop is None or "store::settings()" in screen_loop.group(0):
        failures.append("the screen's context copies the whole settings every few seconds")
    ui = (ROOT / "main" / "ui.cpp").read_text(encoding="utf-8")
    if "store::settings()" in ui:
        failures.append("ui.cpp copies the whole settings")
    for failure in failures:
        print("FAIL", failure)
    print("lvgl config:", "FAILED" if failures else "ok")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
