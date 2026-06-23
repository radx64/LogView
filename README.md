# LogView

**A fast, cross-platform desktop tool for exploring and making sense of large log files.**

LogView lets you open huge logs, drill down with chained filters, highlight what matters, bookmark key lines, and save the whole investigation as a reusable project — so you can pick up right where you left off.

| CI | Status |
|---|---|
| Linux | [![Build Linux](https://github.com/radx64/LogView/actions/workflows/build-linux.yml/badge.svg)](https://github.com/radx64/LogView/actions/workflows/build-linux.yml) |
| Windows | [![Build Windows](https://github.com/radx64/LogView/actions/workflows/build-windows.yml/badge.svg)](https://github.com/radx64/LogView/actions/workflows/build-windows.yml) |

---

## Features

- **Handles large files** — logs are loaded in chunks on a background thread, so the UI stays responsive even with very big files.
- **Hierarchical grep** — filter the current view by a pattern, then filter the result again, building a tree of progressively narrower views. Each filter supports plain text or regex, case sensitivity, and inversion.
- **Find** — quickly jump between matches within a view.
- **Bookmarks** — mark important lines and navigate back to them at any time.
- **Markings & auto-markings** — highlight lines by color, manually or automatically via rules, to make patterns jump out.
- **Merge files** — combine multiple log files into a single chronological view.
- **Export** — save a filtered (grepped) view to a new file.
- **Projects** — save your open files, filter trees, bookmarks, and markings to a project file and reload them later.
- **Themes** — System, Light, and Dark.
- **Built-in updater** — checks GitHub for new releases and notifies you in-app.

## Download

Pre-built binaries for Linux and Windows are published on the
[Releases page](https://github.com/radx64/LogView/releases).

## Building from source

### Requirements

- A C++17 compiler
- [CMake](https://cmake.org/) 3.16+
- [Qt](https://www.qt.io/) 6.6 or newer (Core, Gui, Widgets, Network)

### Build

```bash
./build.sh
```

The script configures and builds a Release binary into `build/LogView`.

Useful overrides:

```bash
BUILD_TYPE=Debug ./build.sh   # debug build
JOBS=4 ./build.sh             # limit parallel jobs
./build.sh clean              # wipe the build directory first
```

Or invoke CMake directly:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Usage

1. **Open a log** via *File → Open file...* (`Ctrl+O`), or merge several with *File → Merge files...*.
2. **Filter** the current view with *Edit → Grep current view* (`Ctrl+G`). Repeat to drill deeper; each grep becomes a child view in the filter tree.
3. **Find** text within a view with `Ctrl+F`.
4. **Bookmark** the current line (`Ctrl+B`) to return to it later.
5. **Save your work** as a project (`Ctrl+S`) and reload it later (`Ctrl+L`).

### Keyboard shortcuts

| Action | Shortcut |
|---|---|
| Open file | `Ctrl+O` |
| Find | `Ctrl+F` |
| Grep current view | `Ctrl+G` |
| Export grep | `Ctrl+E` |
| Bookmark current line | `Ctrl+B` |
| Save project | `Ctrl+S` |
| Save project as | `Ctrl+Shift+S` |
| Load project | `Ctrl+L` |
| Exit | `Ctrl+Q` |

## Releasing

Releases are built and published automatically by GitHub Actions when a `v*`
tag is pushed. See [RELEASING.md](RELEASING.md) for the full process.

## License

LogView is released under the [GNU General Public License v3.0](LICENSE).
