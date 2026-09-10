# copycached

A small memcached clone written in C.

> **Work in progress** — this README describes the full design; not everything is implemented yet.

copycached speaks a text protocol similar to memcached (`set`, `get`, ...) over a single-threaded `poll()` TCP server. It stores keys and opaque byte values in a fixed-size store — no threads, no locks.

It is built as a toy recreational project and designed to be used as a chat backend for small game lobbies with very little chat-specifc code in the server. 

## Build & run
```
make
./copycached
```

Currently runs as a REPL — type `help` to list commands.

The original REPL implementation lives in [`repl-legacy/`](repl-legacy/).

MIT License, 2026, Erik Kranjec
