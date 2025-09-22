# ARK
ARK is an archive format used for asset packs in Arkas Engine games

Integers are little endian

## Header
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 2    | ARK version                                                       |
| 2      | 1    | Unused - must be set to zero                                      |
| 3      | 4    | String table length in bytes                                      |
| 7      | 4    | Random number - set when the archive is created or updated        |

After the header, the string table begins, then the file entries. The first entry is
always the entry for the `/` directory. The path name for this folder is empty.

## String table
After the header, there is the string table. This contains all of the strings,
stored as ASCII strings that are null terminated.

## File entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 1    | Type: 0 = file, 1 = folder                                        |
| 1      | 4    | Size of file, unused if this is a folder                          |
| 5      | 2    | File name offset in string table                                  |
| ?      | ?    | File contents                                                     |

### Directories
Directories are files that contain file entries. Directories have this header:

| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | Number of file entries in directory                               |
