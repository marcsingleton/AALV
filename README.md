# AALV
AALV is Another ALignment Viewer. Unlike the [many other options available](https://en.wikipedia.org/wiki/List_of_alignment_visualization_software), it's a compiled command-line utility, so it can be distributed as a single executable and used from the shell. It supports colorized sequences, multi-file viewing, and reading from streams.

## Installation
Pre-built binaries for select platforms are available as GitHub releases.

Building from source requires Make and a C compiler, *e.g.* GNU or LLVM. AALV targets the C99 standard and uses the standard and curses libraries. These should be available on Unix-like platforms with installed development tools.

Compile with the following commands:

```
git clone https://github.com/marcsingleton/AALV
cd AALV
make
```

The binary can be installed by moving it to a location on `PATH`, *e.g.* `/usr/bin/`.

## Commands
AALV attempts to faithfully mimic Vim-style commands. Currently, Ex ("colon") commands aren't implemented, so those are mapped to keystrokes. As AALV has an alignment-centric interface, there are additional commands to support left/right paging and manipulating the sizes of panes. Here's a short list of key differences and additions:
  - `q`: quit
  - `< / >`: previous/next file
  - `[ / ]`: decrease/increase header pane width
  - `{ / }`: decrease/increase ruler pane height
  - `- / +`: decrease/increase tick spacing
  - `^B / ^F`: page up/down
  - `^U / ^D`: half page up/down
  - `^P / ^N`: page left/right
  - `^L / ^R`: half page left/right

There are a few more not listed here. The brave can refer to `input_parse_keys` in [input.c](src/input.c).

For simplicity some commands differ slightly from Vim's implementation. Here's some of them:
  - Re-sized windows are pinned to the top rather than roughly around the cursor
  - Paging motions do not shift the cursor; they only shift the offset, and the cursor is only moved to keep it within bounds
  - On a failed command sequence the buffer, including any digits, is reset; for example, in AALV `3gj` moves the cursor one row down whereas in Vim it moves the cursor three rows down

## FAQ
### Why is the cursor out of register with the sequences or the UI look strange?
You've likely opened a file that contains non-ASCII encoded characters! AALV should warn you warn you when it detects non-ASCII characters in the sequences, but perhaps you missed it or they're hiding in a header?

For the curious, ASCII encodings fit in a single byte, so there is a direct correspondence between the raw bytes in a sequence and the display width. UTF-8, the most common encoding used today, is variable width, and the relationship between these byte sequences and their display width is, to put it lightly, complex, so AALV politely opts out of supporting it. Fortunately, ASCII is a subset of UTF-8, so this shouldn't be an issue for biological sequences.

### Why is AALV not rendering in color or a reduced number of colors?
There are two possible reasons for this. First, AALV checks if your terminal supports color. Print the `TERM` variable in your shell and make sure it's set to a value that supports color. (Usually `xterm-256color` will do the trick.) Second, AALV checks if the sequences support color rendering. More specifically, they must be ASCII, and they must be recognized as belonging to AALV's nucleic or protein alphabets. You can override the latter check by explicitly providing a type (`-t`) argument.

## Future Development and Contributing
[TODO.md](TODO.md) has a short list of "small" features that could be implemented as weekend projects. Longer term, I would like for AALV to evolve into a full-fledged sequence and alignment editor. This would likely require some fundamental changes in how sequences are represented, among other maintainability improvements like increased test coverage and more consistent use of setters and getters, so it would not be trivial. For now, I'm taking a break from active development, but in the meantime, contributions are welcome! Don't hesitate to reach out with any questions or ideas!
