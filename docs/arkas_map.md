# ARM (ARkas Map)
All integers are little endian

## Header
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 2    | ARM version (currently 1)                                         |
| 0      | 4    | Number of points                                                  |
| 4      | 4    | Number of walls                                                   |
| 8      | 4    | Number of sectors                                                 |
| 12     | 4    | Number of strings                                                 |

After this, there are sets of these entries (in this order):
- String table
- Points
- Walls
- Sectors

## String table entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | Number of characters in string                                    |
| 4      | ?    | String contents                                                   |

## Point entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | X offset (float)                                                  |
| 4      | 4    | Y offset (float)                                                  |

## Wall entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 1    | Boolean - is a portal?                                            |
| 1      | 4    | String table index - wall texture path                            |

If the wall is a portal, it contains these values:
| Size | Description                              |
| ---- | ---------------------------------------- |
| 4    | Wall index for where the portal goes     |
| 4    | X offset (float)                         |
| 4    | Y offset (float)                         |

## Sector entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | Point index - start of sector                                     |
| 4      | 4    | Number of points in index                                         |
| 8      | 4    | Float - height of ceiling                                         |
| 12     | 4    | Float - height of floor                                           |
| 16     | 4    | String table index - floor texture path                           |
| 20     | 4    | String table index - ceiling texture path                         |
