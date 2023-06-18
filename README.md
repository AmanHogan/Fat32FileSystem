# MFS - Miniature File System

This is a simple implementation of a miniature file system called MFS (Miniature File System). It provides basic file system operations such as opening and closing a file, listing files in a directory, reading file contents, and more.

## Prerequisites

To run this program, you need to have a C compiler installed on your system.

## How to Run

1. Clone this repository or download the source code.
2. Compile the code using a C compiler. For example, using GCC: `gcc -o mfs mfs.c`
3. Run the compiled program. For example: `./mfs`

## Command List

- `open <file>`: Open the specified file system image.
- `close`: Close the currently open file system image.
- `ls`: List files in the current directory.
- `info`: Print information about the file system.
- `cd <directory>`: Change the current directory to the specified directory.
- `get <file>`: Copy the specified file from the file system to the current directory.
- `read <file> <position> <bytes>`: Read the specified number of bytes from the specified position in a file.
- `delete <file>`: Delete the specified file.
- `stat <file>`: Print information about the specified file.

## Notes

- The file system image should be in the same directory as the program.
- Only one file system image can be opened at a time.
- Before using any command other than `open` and `close`, make sure to open a file system image using the `open` command.
- The file names in the MFS follow the FAT32 naming convention.
- This implementation assumes a maximum of 16 entries in the root directory.

## Author

- Aman Hogan-Bailey
- Student ID: 1001830469
