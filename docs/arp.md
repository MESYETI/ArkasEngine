# ARkas Project (ARP)
Arkas Project is a file format for map projects. It is very similar to ARM

All integers are little endian

## Header
| Size | Description                     |
| ---- | ------------------------------- |
| 2    | ARP version (currently 0)       |
| 4    | Number of sectors               |
| 4    | Size of string table            |

## String entry
| Size | Description                     |
| ---- | ------------------------------- |
| ?    | Null terminated string          |

## Sector entry
| Size | Description                               |
| ---- | ----------------------------------------- |
| 4    | Float - height of ceiling                 |
| 4    | Float - height of floor                   |
| 4    | String table index - floor texture        |
| 4    | String table index - ceiling texture      |
| 4    | String table index - default wall texture |
| 4    | Number of walls                           |

Then, inside this sector entry, is some number of wall entries

### Wall entry
| Size | Description                               |
| ---- | ----------------------------------------- |
| 4    | Float - start X                           |
| 4    | Float - start Y                           |
| 4    | Portal index (-1 if not portal)           |
| 4    | String table index - wall (-1 if default) |
