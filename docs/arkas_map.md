# ARM (ARkas Map)
All integers are little endian

## Header
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | Number of points                                                  |
| 4      | 4    | Number of walls                                                   |
| 8      | 4    | Number of sectors                                                 |

After this, there are sets of these entries (in this order):
- Points
- Walls
- Sectors

## Point entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | X position (float)                                                |
| 4      | 4    | Y position (float)                                                |

## Wall entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 1    | Boolean - is a portal?                                            |
| 1      | 4    | Wall index for where the portal goes                              |
| 5      | 4    | Number of characters in texture string                            |
| 9      | ?    | Texture path                                                      |

## Sector entry
| Offset | Size | Description                                                       |
| ------ | ---- | ----------------------------------------------------------------- |
| 0      | 4    | Point index - start of sector                                     |
| 4      | 4    | Number of points in index                                         |
| 8      | 4    | Float - height of ceiling                                         |
| 12     | 4    | Float - height of floor                                           |
| 16     | 4    | Number of characters in floor texture string                      |
| 20     | ?    | Floor texture path                                                |
| ?      | 4    | Number of characters in ceiling texture string                    |
| ?      | ?    | Ceiling texture path                                              |
