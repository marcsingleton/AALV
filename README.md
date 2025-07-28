# AALV

AALV is Another ALignment Viewer. Unlike the [many other options available](https://en.wikipedia.org/wiki/List_of_alignment_visualization_software), it's a compiled command-line utility, so it can be distributed as a single executable and used from the shell.

## Commands
AALV attempts to mimic many aspects of Vim-style navigation, but for simplicity some features differ slightly. Here's a short list:
  - Re-sized windows in AALV are pinned to the top rather than roughly around the cursor as in Vim
  - Paging motions don't shift the cursor; they only shift the offset, and the cursor is only moved to prevent it exceeding the last record

## TODO
- More supported file formats
- Status bar w/ file names, total rows and columns, and mechanism for displaying messages
- Fix handling of empty files and empty sequence records
- Allow alphabets to recognize lowercase variants
  - Case-insensitive flag
  - Map all letter-types to upper internally
- Commands
  - Set sequence type?
  - Display options
    - Add color scheme
    - Set color scheme
    - Identities as dots
  - Next/previous/index file
- Simple compound keys
  - Number then movement