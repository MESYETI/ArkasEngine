# Arkas Engine multiplayer protocol packets
Notes:
- Integers are little endian
- Current protocol version is 0
- A ping packet is expected at least 30 seconds. Sending a ping packet every 15 seconds
  is encouraged.

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

#### 0x02 - Chat message
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 128  | Null terminated chat message - one line       |

Notes: A chat message without a null terminator is 128 characters long

### Server to client
#### 0x00 - Identification
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 32   | Server name - null terminated string          |

#### 0x01 - Send file
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 64   | File name                                     |
| 4    | Size of file                                  |
| ?    | File contents                                 |

Notes:
- File name does not include the drive
- The client may have a file size limit
- The file will be stored in the `net:` drive
- If the server sends `map.arm`, the client will load that map
- If the file already exists in the client's memory, the file will be replaced

#### 0x02 - Chat message
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 64   | Null terminated chat message - one line       |

Notes: A chat message without a null terminator is 64 characters long

#### 0x03 - Ping
This packet contains no extra data.

#### 0x04 - Kick
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 256  | Null terminated kick message - one line       |

#### 0xFF - Game packet - currently unsupported
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 4    | Packet size                                   |
| ?    | Packet data                                   |

## Unreliable connection (UDP, or over reliable connection)
All UDP packets contain a 16-bit little endian length at the start, and they may
contain one or more Arkas Engine packets. If the Arkas Engine packets are sent
over a reliable connection, this is not the case

### Client to server
#### 0xFF00 - My current position
| Size | Description                                   |
| ---- | --------------------------------------------- |
| 4    | Float - X position                            |
| 4    | Float - Y position                            |
| 4    | Float - Z position                            |
| 4    | Float - Yaw direction                         |
| 4    | Float - Pitch direction                       |

#### 0x03 - Ping
This packet contains no extra data.
