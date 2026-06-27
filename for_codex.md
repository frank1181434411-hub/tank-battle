# Codex 任务说明：补全 Tank Battle 工作区配置

## 任务目标

请检查当前工作区 `D:\CPP\tank-battle`，仅补全项目开发环境、构建系统、VS Code 工作区和 Git 忽略规则所需的配置文件。

当前阶段不要编写任何游戏业务代码，不要补充说明文档，不要添加图片、音频、地图或其他资源内容。

## 已确认的开发环境

- 操作系统：Windows 10/11 64 位
- 项目路径：`D:\CPP\tank-battle`
- MSYS2 根目录：`D:\msys2`
- 终端环境：MSYS2 UCRT64
- C++ 编译器：`D:\msys2\ucrt64\bin\g++.exe`
- C 编译器：`D:\msys2\ucrt64\bin\gcc.exe`
- 调试器：`D:\msys2\ucrt64\bin\gdb.exe`
- CMake：`D:\msys2\ucrt64\bin\cmake.exe`
- Ninja：`D:\msys2\ucrt64\bin\ninja.exe`
- Git：MSYS2 内的 `/usr/bin/git`
- C++ 标准：C++17
- 图形、音频和网络库：SFML 3.1.0
- SFML 获取方式：CMake `FetchContent`
- VS Code 扩展：
  - `ms-vscode.cpptools`
  - `ms-vscode.cmake-tools`
  - `twxs.cmake`

## 必须遵守的限制

1. 不要修改任何现有 `.cpp`、`.hpp`、`.h` 文件内容。
2. 不要创建或补充任何游戏逻辑代码。
3. 不要修改或补充 `README.md`。
4. 不要创建需求分析、设计报告、规则书、测试报告等说明文档。
5. 不要添加任何图片、音频、字体、地图或模型文件。
6. 不要安装软件，不要执行 `pacman`。
7. 不要修改 Windows 全局环境变量。
8. 不要修改 Git 用户名、邮箱、远程仓库和分支。
9. 不要执行 `git commit` 或 `git push`。
10. 不要删除用户已有文件。
11. 对已有配置文件先读取并合并，不要无条件覆盖。
12. 不要创建 `c_cpp_properties.json`，除非 CMake Tools 无法提供 IntelliSense 配置。
13. 不要创建 `tasks.json`，项目统一使用 CMake Tools 和 CMake Presets。
14. 所有路径必须基于 `D:\msys2`，不能写成 `C:\msys64`、`D:\msys64` 或其他路径。
15. 所有文本文件使用 UTF-8 和 LF 换行。
16. 不要提交或跟踪 `build/`、编译产物、SFML 下载缓存和 IDE 缓存。

## 需要检查并补全的文件

请检查以下文件是否存在、是否为空、是否配置错误，并按要求创建或修正：

```text
D:\CPP\tank-battle
├── CMakeLists.txt
├── CMakePresets.json
├── .gitignore
├── .gitattributes
├── .editorconfig
└── .vscode
    ├── settings.json
    ├── launch.json
    └── extensions.json
```

不要创建其他配置文件，除非当前工作区确实需要，并在最终报告中说明原因。

---

# 一、CMakeLists.txt 要求

如果 `CMakeLists.txt` 不存在或为空，请创建最小可扩展版本。

如果已经存在，请保留其中合理内容并修正错误。

必须满足：

1. `cmake_minimum_required(VERSION 3.28)`。
2. 项目名为 `TankBattle`。
3. 使用 C++17：
   - `CMAKE_CXX_STANDARD 17`
   - `CMAKE_CXX_STANDARD_REQUIRED ON`
   - `CMAKE_CXX_EXTENSIONS OFF`
4. 导出 `compile_commands.json`。
5. 可执行文件输出到 `${CMAKE_BINARY_DIR}/bin`。
6. 通过 `FetchContent` 获取 SFML 3.1.0。
7. SFML 仓库：
   - `https://github.com/SFML/SFML.git`
   - `GIT_TAG 3.1.0`
8. 链接：
   - `SFML::Graphics`
   - `SFML::Audio`
   - `SFML::Network`
9. 包含目录：
   - `${CMAKE_CURRENT_SOURCE_DIR}/include`
10. 编译警告：
    - `-Wall`
    - `-Wextra`
    - `-Wpedantic`
11. 只把当前工作区中真实存在的 `.cpp` 文件加入目标。
12. 不要自动生成、修改或补全任何源文件。
13. 不要使用 `file(GLOB_RECURSE ...)`。
14. 如果 `assets/` 或 `config/` 存在，使用带 `if(EXISTS ...)` 保护的 `POST_BUILD` 命令复制到可执行文件目录。
15. 定义以下选项，默认关闭：
    - `TANK_BATTLE_BUILD_TESTS`
    - `TANK_BATTLE_ENABLE_RL`
    - `TANK_BATTLE_ENABLE_DEBUG_UI`
16. 当前阶段不要为 `tests/` 创建测试目标，除非其中已有可编译的完整测试代码。
17. 不要添加 Qt、SDL、raylib、Boost、pybind11、LibTorch 或其他第三方依赖。
18. 不要下载或链接 MSYS2 仓库中的 SFML 包，统一使用 `FetchContent`。

注意：当前源文件可能仍为空，因此允许 CMake 配置成功但最终链接失败。不要为了让链接成功而向 `main.cpp` 写入代码。

---

# 二、CMakePresets.json 要求

创建或修正 `CMakePresets.json`。

要求：

1. 使用 Ninja。
2. 使用绝对工具路径：
   - `D:/msys2/ucrt64/bin/gcc.exe`
   - `D:/msys2/ucrt64/bin/g++.exe`
   - `D:/msys2/ucrt64/bin/ninja.exe`
3. 在环境变量 `PATH` 前部加入：
   - `D:/msys2/ucrt64/bin`
   - `D:/msys2/usr/bin`
4. 创建隐藏基础 Preset：`ucrt64-base`。
5. 创建配置 Preset：
   - `debug`
   - `release`
6. 创建构建 Preset：
   - `debug`
   - `release`
7. 构建目录：
   - Debug：`${sourceDir}/build/debug`
   - Release：`${sourceDir}/build/release`
8. Debug 配置：
   - `CMAKE_BUILD_TYPE=Debug`
   - `TANK_BATTLE_ENABLE_DEBUG_UI=ON`
   - `TANK_BATTLE_ENABLE_RL=OFF`
   - `TANK_BATTLE_BUILD_TESTS=OFF`
9. Release 配置：
   - `CMAKE_BUILD_TYPE=Release`
   - 三个项目选项全部为 `OFF`
10. 开启 `CMAKE_EXPORT_COMPILE_COMMANDS`。
11. 不要写入个人用户名、代理地址或其他机器相关信息。

---

# 三、VS Code settings.json 要求

创建或修正 `.vscode/settings.json`。

必须包含：

```json
{
    "cmake.useCMakePresets": "always",
    "cmake.cmakePath": "D:/msys2/ucrt64/bin/cmake.exe",
    "cmake.sourceDirectory": "${workspaceFolder}",
    "cmake.configureOnOpen": false,
    "cmake.buildBeforeRun": true,
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "C_Cpp.intelliSenseEngine": "default"
}
```

另外配置工作区默认终端为 MSYS2 UCRT64，启动命令基于：

```text
D:\msys2\msys2_shell.cmd -defterm -here -no-start -ucrt64
```

统一编辑器设置：

- UTF-8
- LF
- 4 空格缩进
- 禁止自动检测缩进
- 暂时关闭保存时自动格式化

如果文件中已有 CPH、主题、字体或其他与本项目无冲突的用户配置，应尽量保留。

---

# 四、VS Code launch.json 要求

创建或修正 `.vscode/launch.json`。

要求：

1. 调试类型：`cppdbg`
2. 请求类型：`launch`
3. 程序路径：
   - `${command:cmake.launchTargetPath}`
4. 工作目录：
   - `${workspaceFolder}`
5. 调试器：
   - `D:\msys2\ucrt64\bin\gdb.exe`
6. `MIMode`：
   - `gdb`
7. 运行时 PATH 前部加入：
   - `D:\msys2\ucrt64\bin`
8. 开启 GDB pretty printing。
9. 不写死 `TankBattle.exe` 的具体构建路径。
10. 不写死 Debug 或 Release 目录。
11. 不添加与当前项目无关的调试配置。

---

# 五、VS Code extensions.json 要求

创建或修正 `.vscode/extensions.json`。

推荐扩展必须包含：

```json
{
    "recommendations": [
        "ms-vscode.cpptools",
        "ms-vscode.cmake-tools",
        "twxs.cmake"
    ]
}
```

不要删除文件中已有的 CPH 推荐扩展；如已存在，合并并去重。

---

# 六、.gitignore 要求

创建或修正根目录 `.gitignore`。

必须忽略：

```gitignore
# Build
build/
out/
cmake-build-*/

# CMake
CMakeCache.txt
CMakeFiles/
cmake_install.cmake
compile_commands.json
build.ninja
.ninja_deps
.ninja_log

# Compiled files
*.o
*.obj
*.a
*.lib
*.dll
*.exe
*.pdb

# VS Code generated cache
.vscode/ipch/
.vscode/.browse.VC.db*
.vscode/*.log

# Python and RL
rl/.venv/
rl/__pycache__/
*.pyc
*.pyo

# Runtime output
logs/
screenshots/
crash_dumps/

# OS files
.DS_Store
Thumbs.db
desktop.ini

# Temporary and backup files
*.tmp
*.bak
*.swp
*~
```

重要：

1. 不要忽略整个 `.vscode/`。
2. 以下文件需要提交：
   - `.vscode/settings.json`
   - `.vscode/launch.json`
   - `.vscode/extensions.json`
3. 不要忽略 `assets/`、`config/`、`docs/`、`include/`、`src/` 和 `tests/`。
4. 不要忽略 `CMakePresets.json`。
5. 不要忽略 `for_codex.md`。

---

# 七、.gitattributes 要求

创建或修正 `.gitattributes`。

要求：

1. 默认文本文件自动规范化。
2. C++、CMake、JSON、Markdown、INI 使用 LF。
3. 图片、音频、字体和模型文件按二进制处理。

建议至少包含：

```gitattributes
* text=auto

*.cpp text eol=lf
*.hpp text eol=lf
*.h text eol=lf
*.c text eol=lf
*.cmake text eol=lf
CMakeLists.txt text eol=lf
*.json text eol=lf
*.md text eol=lf
*.ini text eol=lf
*.txt text eol=lf
*.gitignore text eol=lf

*.png binary
*.jpg binary
*.jpeg binary
*.gif binary
*.wav binary
*.ogg binary
*.mp3 binary
*.ttf binary
*.otf binary
*.pt binary
*.onnx binary
```

---

# 八、.editorconfig 要求

创建或修正 `.editorconfig`。

要求：

```ini
root=true

[*]
charset=utf-8
end_of_line=lf
insert_final_newline=true
trim_trailing_whitespace=true

[*.{cpp,hpp,h,c,cxx,hxx}]
indent_style=space
indent_size=4

[*.{json,cmake,txt,md,ini}]
indent_style=space
indent_size=4
```

不要在 `.editorconfig` 中加入个人偏好或与项目无关的规则。

---

# 九、执行和验证要求

完成配置后，只执行不会修改业务代码的检查。

依次执行：

```bash
cd /d/CPP/tank-battle
```

检查工具：

```bash
echo $MSYSTEM
which g++
which gdb
which cmake
which ninja
```

预期：

```text
UCRT64
/ucrt64/bin/g++
/ucrt64/bin/gdb
/ucrt64/bin/cmake
/ucrt64/bin/ninja
```

检查 CMake Preset：

```bash
cmake --list-presets
```

检查 JSON 语法和文件存在性。

删除旧的 Debug 缓存后配置：

```bash
rm -rf build/debug
cmake --preset debug
```

限制：

1. 如果 `cmake --preset debug` 成功，记录结果。
2. 不要强制执行完整构建。
3. 如果源文件为空导致构建或链接失败，不要补写代码。
4. 如果 SFML 下载失败，只报告网络错误，不要更换 SFML 版本。
5. 不要执行 Release 构建，除非 Debug 配置已经成功。
6. 不要执行游戏程序。
7. 不要执行 Git 提交或推送。

---

# 十、最终输出要求

完成后给出简洁报告，必须包含：

1. 新建了哪些文件。
2. 修改了哪些已有文件。
3. 保留了哪些原有配置。
4. CMake 是否成功识别：
   - GNU C++ 编译器
   - Ninja
   - SFML 3.1.0
5. `cmake --preset debug` 是否成功。
6. 如果失败，给出完整错误原因。
7. 列出 `git status --short` 的结果。
8. 明确说明没有修改任何游戏业务代码。
9. 明确说明没有执行 `git commit` 和 `git push`。

开始执行前，请先读取当前工作区，确认实际文件状态，再进行最小必要修改。
