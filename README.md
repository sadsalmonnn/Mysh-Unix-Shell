# mysh — A Custom Unix Shell Implementation

## Overview

`mysh` is a simple Unix-like shell implemented in C, supporting:

- Command parsing and execution
- Builtin commands and background processes
- Piping and redirection
- Environment variable management
- Signal handling (e.g., Ctrl+C)
- Networking support (Milestone 5) — chat server/client

---

## Features

### 1. Command Parsing and Execution

- Reads user input and tokenizes commands
- Supports execution of system binaries and builtin commands
- Handles background execution with `&`
- Supports command pipelines using `|`

### 2. Builtin Commands

Includes builtins such as `exit`, `cd`, `start-server`, `close-server`, `send`, and `start-client`.

### 3. Signal Handling

Graceful handling of `SIGINT` (Ctrl+C) to prevent shell termination.

### 4. Environment Variables

Linked-list based environment variable storage and lookup with commands to manipulate them.

### 5. Networking Support

- **start-server port-number**  
  Launches a non-blocking background chat server on the specified port, accepting multiple simultaneous client connections.  
  *Errors:* Prints `ERROR: No port provided` if port is missing.

- **close-server**  
  Terminates the running chat server gracefully.

- **send port-number hostname message**  
  Sends a message to a given hostname and port. Messages appear on the shell's console and all connected clients.

- **start-client port-number hostname**  
  Starts a client that connects once to the server at the specified port and hostname.  
  The client can send multiple messages until terminated with Ctrl+D or Ctrl+C.  
  Upon connecting, each client receives a unique ID in the format `client#:`.  
  Messages sent by clients are displayed on the server shell and all connected clients.  
  Special message `\connected` reports the current number of connected clients.  
  *Errors:* Prints `ERROR: No port provided` or `ERROR: No hostname provided` if arguments are missing.

---

## Usage

Launch the shell executable:

```bash
./mysh
