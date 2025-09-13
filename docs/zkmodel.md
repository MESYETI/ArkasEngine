# ZKModel format
ZKModel is a model format made by me

All fixed point numbers are little endian, and have 16 bit precision

All integers are little endian

## Header
| Field name   | Field size (bytes) | Field description                               |
| ------------ | ------------------ | ----------------------------------------------- |
| Magic bytes  | 3                  | Contains 'ZKM' as an ASCII string               |
| Version      | 1                  | 8-bit integer containing version (currently 0)  |
| Vertices num | 4                  | 32 bit integer equal to the amount of vertices  |
| Faces num    | 4                  | 32 bit integer equal to the amount of faces     |

## Vertices section

| Field name   | Field size (bytes) | Field description                                               |
| ------------ | ------------------ | --------------------------------------------------------------- |
| Coordinates  | 12                 | 3 32-bit floating point numbers with the position of the vertex |

## Faces section

| Field name   | Field size (bytes) | Field description                               |
| ------------ | ------------------ | ----------------------------------------------- |
| Vertices     | 6                  | 3 16-bit indexes for vertices in the face       |
| Colour       | 3                  | The colour of the face                          |
