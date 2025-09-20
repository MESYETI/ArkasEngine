# ARK
ARK is an archive format used for asset packs in Arkas Engine games

Integers are little endian

## Header
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 2    | ARK version                                                       |
| 2      | 1    | Unused - must be set to zero                                      |
| 3      | 4    | Total number of file entries                                      |
| 7      | 4    | Offset of file contents in file                                   |

After the header, the file entries begin. The first one is always the entry for the `/`
directory. The path name for this folder is empty.

## File entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 1    | Type: 0 = file, 1 = folder                                        |
| 1      | 4    | Size of file                                                      |
| 5      | 4    | Offset of file contents                                           |
| 7      | 2    | Length of file name                                               |
| 9      | ?    | File name                                                         |

### Directories
Directories are files that contain file entries. Directories have this header:

| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | Number of file entries in directory                               |

## File contents
After the file entries the contents of every file are written here.

All file contents must begin with 4 bytes that equal `FILE` in ASCII.
