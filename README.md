# UDP Chat Room

UDP broadcast chat in C.

## Build
```bash
cl server.c ws2_32.lib -o server.exe
cl client.c ws2_32.lib -o client.exe
```

## Run
```bash
# Terminal 1
./server.exe

# Terminal 2-N
./client.exe
```

## Commands
- `/nick <name>` - Set nickname
- `/msg <text>` - Send message
- `/users` - Show online users
- `/quit` - Disconnect

## Example
```
> /nick Alice
Welcome to the chat!

> /msg Hello!
[Alice]: Hello!
```
