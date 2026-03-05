# CodeClipper

 Utility designed to gather source code context from directories.

## Features

*   📁 **Recursive Traversal**: Scans your project directory efficiently.
*   🚫 **Gitignore**: Automatically respects rules from `.gitignore` files to skip build artifacts and dependencies.
*   📋 **Clipboard Integration**: Copies the formatted output directly to your system clipboard (can be disabled with `--no-copy`).
*   🤖 **MCP Server**: Implements the Model Context Protocol, allowing AI tools to use CodeClipper as a tool to explore your local code.
*   🛡️ **Hidden File Protection**: Skips hidden directories (like `.git` or `.vscode`) by default unless specified.

## Prerequisites

*   **CMake**: 3.20 or higher.
*   **C++ Compiler**: Supporting C++20 (GCC 10+, Clang 11+, or MSVC 19.29+).
*   **Git**: Required for fetching dependencies during build.

## Building

To build release version from source follow commands:

```bash
cmake . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build
```

Then install it:
```bash
cmake --install build --prefix <path>
```

### Build with Sanitizers (Debug)

To enable Address and Undefined Behavior sanitizers for development (GCC/Clang):

```bash
cmake . -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON -B build_debug
cmake --build build_debug
```

### Running Tests

If `BUILD_TESTS` is enabled (it is `ON` by default):

```bash
cd build
ctest --output-on-failure
```

## Usage

### Basic Usage
Gather all files in the current directory and copy them to the clipboard:
```bash
cclip
```

### Specifying a Path
```bash
cclip /path/to/your/project
```

### Filtering Examples
Include only header files and ignore a specific legacy folder:
```bash
cclip -o "*.hxx" -i "legacy/"
```

### Console Output Only
Print the Markdown to the terminal without affecting the clipboard:
```bash
cclip --no-copy
```

### MCP Mode
To use CodeClipper as an MCP server for AI tools, start it with the `--mcp` flag:
```bash
cclip --mcp
```

## CLI Options

| Flag | Description |
|---|---|
| `path` | Positional argument: Root directory to scan (Default: `.`) |
| `-m, --mcp` | Run as an MCP (Model Context Protocol) Server |
| `-i, --ignore` | Extra glob patterns to ignore |
| `-o, --only` | Process ONLY these file patterns (whitelist) |
| `--no-copy` | Output to console only, do not touch clipboard |
| `-l, --list` | List file paths only, do not copy content |
| `-v, --verbose` | Enable verbose logging |
| `--no-gitignore` | Ignore `.gitignore` files and process all files |

## License
Copyright 2026 dand104

Licensed under the Apache License, Version 2.0; See LICENSE
