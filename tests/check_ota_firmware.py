#!/usr/bin/env python3
# ARMOR-NODE-HMI - guard on the firmware upload of the panel's server. Nothing is compiled or flashed: main/web_server.cpp and the files of the boards are read, and this fails when
#   1. the node has more than one board (the other four nodes run one project on two boards and carry a mark of the board inside every image; this one has one board, lcd7box, and
#      checks the name of the project instead: a second board would make that check not enough, and then the mark of core/image_board.hpp has to be ported);
#   2. the upload does not answer 422 "wrong_firmware" for an image of another node before esp_ota_set_boot_partition(target);
#   3. the switch to the other slot does not look at the project of that slot before esp_ota_set_boot_partition(other);
#   4. a failed or interrupted write does not abort the update handle.
# A damaged copy of the source is also tried for every guard, so the guard is shown to fail. Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def read(relative: str) -> str:
    path = ROOT / relative
    return path.read_text(encoding="utf-8", errors="replace") if path.is_file() else ""


def boards_of(root: Path) -> list[str]:
    return sorted(path.name.removeprefix("sdkconfig.board.") for path in root.glob("sdkconfig.board.*"))


def problems_of(source: str, boards: list[str], has_mark: bool) -> list[str]:
    if not source:
        return ["main/web_server.cpp is missing"]
    problems: list[str] = []
    if len(boards) != 1 and not has_mark:
        problems.append(f"the node has {len(boards)} boards ({', '.join(boards) or 'none'}) and no core/image_board.hpp: the project name does not tell two boards apart, port the mark of the board from the other nodes")
    begin = source.find("esp_ota_begin(target")
    if begin < 0:
        return problems + ["the upload no longer opens the update with esp_ota_begin(target"]
    boot = source.find("esp_ota_set_boot_partition(target", begin)
    if boot < 0:
        return problems + ["the upload no longer sets the new slot with esp_ota_set_boot_partition(target"]
    upload = source[begin:boot]
    close = upload.find("esp_ota_end(")
    names = upload.find("project_name")
    refuse = upload.find('send_error(r, 422, "wrong_firmware")')
    if close < 0:
        problems.append("the upload does not close the image with esp_ota_end before it is judged")
    if names < 0 or refuse < 0:
        problems.append('the upload does not compare the project name of the image and answer 422 "wrong_firmware" for an image of another node')
    if min(close, names, refuse) >= 0 and not close < names < refuse:
        problems.append("the order of the upload must be: close the image, compare the project name, refuse another node, and only then set the boot slot")
    if upload.count("esp_ota_abort(handle)") < 2:
        problems.append("a failed write or an interrupted upload must call esp_ota_abort(handle)")
    if '"not_firmware"' not in upload:
        problems.append('a file that does not start with the magic byte of an image must be answered 422 "not_firmware"')
    switch = source.find("esp_ota_set_boot_partition(other")
    if switch < 0:
        problems.append("the switch to the other slot no longer uses esp_ota_set_boot_partition(other")
    else:
        head = source.rfind("esp_err_t post_ota_switch(", 0, switch)
        part = source[head:switch] if head >= 0 else ""
        if "project_name" not in part or 'send_error(r, 422, "wrong_firmware")' not in part:
            problems.append('the switch to the other slot does not compare the project of that slot and answer 422 "wrong_firmware" before setting it')
    return problems


def damaged(source: str) -> list[tuple[str, str]]:
    """Copies of the source with one guard taken out; every one of them has to be refused."""
    return [
        ("another node not refused on upload", source.replace('return send_error(r, 422, "wrong_firmware");\n  if (esp_ota_set_boot_partition(target)', "(void)0;\n  if (esp_ota_set_boot_partition(target)", 1)),
        ("no comparison of the project", source.replace("std::strcmp(description.project_name, esp_app_get_description()->project_name) != 0) return send_error(r, 422, \"wrong_firmware\");\n  if (esp_ota_set_boot_partition(target)", "false) return send_error(r, 422, \"wrong_firmware\");\n  if (esp_ota_set_boot_partition(target)", 1)),
        ("other slot not looked at", source.replace("if (std::strcmp(description.project_name, esp_app_get_description()->project_name) != 0) return send_error(r, 422, \"wrong_firmware\");\n  if (esp_ota_set_boot_partition(other)", "if (esp_ota_set_boot_partition(other)", 1)),
        ("write not aborted", source.replace("esp_ota_abort(handle)", "(void)handle")),
        ("not firmware not refused", source.replace('"not_firmware"', '"x"', 1)),
    ]


def main() -> int:
    source = read("main/web_server.cpp")
    boards = boards_of(ROOT)
    has_mark = (ROOT / "core" / "image_board.hpp").is_file()
    problems = problems_of(source, boards, has_mark)
    for name, text in damaged(source):
        if text == source:
            problems.append(f"self-test '{name}': the pattern to damage was not found, the guard cannot prove itself")
        elif not problems_of(text, boards, has_mark):
            problems.append(f"self-test '{name}': a damaged server was accepted")
    if len(boards) == 1 and problems_of(source, ["a", "b"], False) == problems_of(source, boards, has_mark):
        problems.append("self-test 'second board': a second board without the mark of the board was accepted")
    if problems:
        print("OTA_FIRMWARE=FAIL")
        for problem in problems:
            print("  " + problem)
        return 1
    print(f"OTA_FIRMWARE=PASS one board ({boards[0] if boards else '-'}), the upload and the switch refuse the firmware of another node before the boot slot is set")
    return 0


if __name__ == "__main__":
    sys.exit(main())
