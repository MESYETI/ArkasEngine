# Arkas Engine multiplayer protocol packets
Notes:
- Integers are little endian
- Current protocol version is 0

Arkas Engine multiplayer requires that a connection is available where data is
reliably sent in order, such as TCP or the local connection. UDP may be used for
unreliably sending data (see the unreliable connection section). In cases
where there is no unreliable connection available (such as on a local connection),
the same packets may be sent over the reliable connection.

## Reliable connection (TCP, local)
Each packet starts with an ID in a 16 bit integer

### Client to server
#### 0x00 - Identification
This WILL change in a future version of the protocol

| Size | Description                                   |
| ---- | --------------------------------------------- |
| 2    | Integer - protocol version                    |
| 32   | Username - null terminated string             |

### Server to client
#### 0x00 - Identification
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 32   | Server name - null terminated string          |

#### 0x01 - Map
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 1    | Length of map name                            |
| ?    | Map name                                      |
| 4    | Size of map                                   |
| ?    | Map data (ARM file)                           |

On the client, the map data is stored in the `net_map:` RAM drive. The client may
have a map size limit

#### 0x02 - Game packet
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 4    | Packet size                                   |
| ?    | Packet data                                   |

## Unreliable connection (UDP, or over reliable connection)
All UDP packets contain a 16-bit little endian length at the start, and they may
contain one or more Arkas Engine packets. If the Arkas Engine packets are sent
over a reliable connection, this is not the case

There are currently no packets to be sent over an unreliable connection.
