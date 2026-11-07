# Entity
This file documents entity serialisation. For debug purposes, an entity may be stored in an `.aent`
file.

## Header
| Size | Description                 |
| ---- | --------------------------- |
| 4    | Entity type ID              |
| 4    | Float - X position          |
| 4    | Float - Y position          |
| 4    | Float - Z position          |
| 4    | Sector ID                   |
| 4    | Float - pitch               |
| 4    | Float - yaw                 |
| 4    | Float - roll                |

In total, the header is 32 bytes.

The data contains however many components the entity has. See `entity.h` to see how the
built-in entities are serialised, and see the game's source code to see how its entity
components are serialised.

## Notes
- Entity components may not be bigger than 1024 bytes.
