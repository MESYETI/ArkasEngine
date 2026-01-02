# ART (ARkas Texture)
ART is an image format used for the Arkas Engine.

# Header
| Value                | Size (bytes) |
| -------------------- | ------------ |
| Magic (ART in ASCII) | 3            |
| Type                 | 1            |
| Width                | 2            |
| Height               | 2            |

# Types
## 0x03
24bpp RGB data

## 0x04
32bpp RGBA data

## 0x01
8bpp greyscale data

## 0x02
16bpp - 8 for greyscale and 8 for alpha

## 0x11
8bpp paletted image

Adds the following fields to the header:

| Value                | Size (bytes) |
| -------------------- | ------------ |
| Palette size - 1     | 1            |

RGB data for the palette follows the header

## 0x21
Same as 0x11 except the palette uses RGBA data
