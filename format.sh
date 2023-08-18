#!/usr/bin/env bash

find . -not \( -path '*/venv/*' -o -path '*/build/*' -o -path '*/cmake-build-debug/*' \) | grep ".*\.\(cpp\|hpp\|cu\|c\|cc\|h\)$" | xargs clang-format --style=file -i
