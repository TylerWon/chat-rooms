# Chat Rooms

## Overview

This is a simple real-time chat application that allows multiple users to communicate in chat rooms. Each user can 
choose a name, join a room, send and receive messages, and exit when they’re done. Messages are timestamped so 
participants can follow the conversation context.

## Features

- Set your name to identify yourself in conversations.
- Join a specific chat room to connect with others.
- Send messages to everyone in the room.
- See messages from other participants, along with the time they were sent.
- Exit the application cleanly.

## Set-up

1. Build the client and server by running `make`
2. Start the server: `./server`
3. Start one or more clients: `./client`

## Client commands

`/join [room number]` - join room `[room number]`
- Example: `/join 5` to join room 5

`/name [name]` - set your name to `[name]`
- Example: `/name tyler` to set your name to `tyler`

`/exit` - close the application
