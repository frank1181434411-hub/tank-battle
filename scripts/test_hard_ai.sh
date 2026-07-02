#!/usr/bin/env bash
set -e

cd "$(dirname "$0")/.."

D:/msys2/ucrt64/bin/cmake.exe --preset debug -DTANK_BATTLE_BUILD_TESTS=ON
D:/msys2/ucrt64/bin/cmake.exe --build --preset debug --target TankBattleAITest
./build/debug/bin/TankBattleAITest.exe
