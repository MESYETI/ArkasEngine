# ARF (Arkas Font)
ARF is a font format that aims to eventually be able to contain multiple kinds
of fonts, but for now, it only contains a PNG.

Integers are little endian

## Header
| Offset | Size | Description      |
| ------ | ---- | ---------------- |
| 0      | 2    | ARF version (0)  |
| 2      | 2    | Font type        |
| 4      | 1    | Character width  |
| 5      | 1    | Character height |

## Font type 0 - PNG
What follows is just a PNG image
