# ZKModel format
ZKModel is a model format

All integers are little endian

## Header
| Field name   | Field size (bytes) | Field description                                |
| ------------ | ------------------ | ------------------------------------------------ |
| Magic bytes  | 3                  | Contains 'ZKM' as an ASCII string                |
| Version      | 1                  | 8-bit integer containing version (currently 1)   |
| Vertices num | 4                  | 32 bit integer equal to the amount of vertices   |
| Faces num    | 4                  | 32 bit integer equal to the amount of faces      |
| UV num       | 4                  | 32 bit integer equal to amount of UV coordinates |
| Texture num  | 4                  | 32 bit integer equal to amount of textures       |
| Flags        | 4                  | Model file flags                                 |

* Flags added in version 1.

### Flags
| Bits (inclusive) | Description                                       |
| ---------------- | ------------------------------------------------- |
| 0                | 0 = no textures, 1 = textures present             |

## Vertices section
| Field name   | Field size (bytes) | Field description                                               |
| ------------ | ------------------ | --------------------------------------------------------------- |
| Coordinates  | 12                 | 3 32-bit floating point numbers with the position of the vertex |

## Faces section
| Field name    | Field size (bytes) | Field description                                  |
| ------------- | ------------------ | -------------------------------------------------- |
| Vertices      | 12                 | 3 32-bit indexes for vertices in the face          |
| 1st UV        | 4                  | First UV coordinate index                          |
| 2nd UV        | 4                  | Second UV coordinate index                         |
| 3rd UV        | 4                  | Third UV coordinate index                          |
| Normal X      | 4                  | Normal vector X                                    |
| Normal Y      | 4                  | Normal vector Y                                    |
| Normal Z      | 4                  | Normal vector Z                                    |
| 2nd normal    | 12                 | Last 3 entries repeated for the 2nd normal vector  |
| 3rd normal    | 12                 | Ditto                                              |
| Texture index | 4                  | Texture index                                      |

* UV coordinates, normal vector, and texture added in version 1.

## UV section
This section was added in version 1. It defines UV coordinates

| Field name    | Field size (bytes) | Field description                                  |
| ------------- | ------------------ | -------------------------------------------------- |
| U             | 4                  | 32-bit float                                       |
| V             | 4                  | 32-bit float                                       |

## Texture section
This section only exists if the "textures present" flag is set to 1. This section
was added in version 1.

| Field name   | Field size (bytes) | Field description                               |
| ------------ | ------------------ | ----------------------------------------------- |
| Type         | 1                  | Texture type                                    |
| Size         | 4                  | Size of texture (not present if type is link)   |

Types:
| Value | Description               |
| ----- | ------------------------- |
| 0     | Link                      |
| 1     | Embedded PNG              |
| 2     | Embedded ART              |

If the type is a Link, then a null-terminated string containing a path follows
the header. If it's an embedded file, then the contents of that file follow the
header
