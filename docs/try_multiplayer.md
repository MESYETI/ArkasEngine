# Try multiplayer!

This is a guide on how to try out multiplayer on standalone Arkas engine.

## Server
Create a file named `server.cmd`. Type the following commands to configure the server:

- `set server.inet true` - Set the server to listen on network sockets
- `set server.inet-port 2025` - Set the server's port to 2025

Then, type `start-server` to start the server.

Optionally, write `map MAP_NAME` to select a map from the `maps` folder.

To run the server, run `arkas --server` in the shell.

## Client
Type `set client.username MY_NAME` to set your username.

Type `start-inet-client IP PORT` to connect to a server.

If the server sends you a map, type `map-view` to view the map.

To use chat, use `chat "MESSAGE"` to send a message. Any incoming messages will be
displayed in the console.
