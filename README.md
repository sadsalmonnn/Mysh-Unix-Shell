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

Starts a client that connects once to the server at the specified port and hostname, where messages can be sent and displayed

---

## Usage

Launch the shell executable:

```bash
./mysh
