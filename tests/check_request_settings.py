#!/usr/bin/env python3
# ARMOR-NODE-HMI - guard on the settings read by the requests of the panel's server. Nothing is compiled: the sources are read, and this fails when
#   1. a request handler of main/web_server.cpp copies the whole settings (about forty texts, some of them kilobytes long) with store::settings() instead of reading the few fields it needs
#      under the lock with store::with_settings; the only handler allowed to copy is the one-time set-up, which changes a copy and saves it;
#   2. main/api_shared.cpp copies the whole settings anywhere but into a heap object (new (std::nothrow) config::Settings(...)), because the web task has a small stack;
#   3. the check on the network that may use the panel (it runs on EVERY request) stops reading the setting under the lock.
# A damaged copy of the sources is also tried, so the guard is shown to fail.
# Copyright (C) 2026 JuanenRac (Electro Hobby 3D). GPL-3.0-or-later.
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ALLOWED_COPY = {"post_setup"}


def read(relative: str) -> str:
    path = ROOT / relative
    return path.read_text(encoding="utf-8", errors="replace") if path.is_file() else ""


def enclosing(source: str, position: int) -> str:
    found = "?"
    for match in re.finditer(r"^(?:esp_err_t|std::string|unsigned|bool|void)\s+(\w+)\(", source[:position], re.M):
        found = match.group(1)
    return found


def problems_of(web: str, shared: str) -> list[str]:
    problems: list[str] = []
    if not web or not shared:
        return ["main/web_server.cpp or main/api_shared.cpp is missing"]
    for match in re.finditer(r"store::settings\(\)", web):
        handler = enclosing(web, match.start())
        if handler not in ALLOWED_COPY:
            problems.append(f"main/web_server.cpp: {handler} copies the whole settings; read the fields it needs with store::with_settings")
    for match in re.finditer(r"store::settings\(\)", shared):
        before = shared[max(0, match.start() - 60):match.start()]
        if "new (std::nothrow) config::Settings(" not in before:
            problems.append(f"main/api_shared.cpp: {enclosing(shared, match.start())} copies the whole settings on the stack; put it on the heap or read the fields it needs")
    if not re.search(r"with_settings\([^;]*adminsrc::allowed\(source, s\.admin_sources\)", web):
        problems.append("the check of the networks that may use the panel no longer reads admin_sources under the lock")
    return problems


def main() -> int:
    web, shared = read("main/web_server.cpp"), read("main/api_shared.cpp")
    problems = problems_of(web, shared)
    tries = {
        "a handler copies the settings": (web.replace("store::with_settings([&](const config::Settings& s) { limit = s.login_limit_per_minute; });", "limit = store::settings().login_limit_per_minute;", 1), shared),
        "the source check copies the settings": (web.replace("store::with_settings([&](const config::Settings& s) { allowed = adminsrc::allowed(source, s.admin_sources); });", "allowed = adminsrc::allowed(source, store::settings().admin_sources);", 1), shared),
        "the status copies the settings on the stack": (web, shared.replace("store::with_settings([&](const config::Settings& s) { node_id = s.node_id;", "const config::Settings s = store::settings(); { node_id = s.node_id;", 1)),
    }
    for name, (w, s) in tries.items():
        if (w, s) == (web, shared):
            problems.append(f"self-test '{name}': the pattern to damage was not found, the guard cannot prove itself")
        elif not problems_of(w, s):
            problems.append(f"self-test '{name}': a damaged source was accepted")
    if problems:
        print("REQUEST_SETTINGS=FAIL")
        for problem in problems:
            print("  " + problem)
        return 1
    print("REQUEST_SETTINGS=PASS the requests read the few fields they need under the lock and copy the whole settings only to change them or on the heap")
    return 0


if __name__ == "__main__":
    sys.exit(main())
