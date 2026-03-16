# C_Shell

A custom, fully-featured UNIX shell written entirely in C from scratch. `C_Shell` is designed to provide a robust command-line interface, complete with process group management, robust signal handling, job control, input/output redirection, and complex pipelining.

## Key Features

### 1. Process & Job Control
- **Foreground & Background Execution:** Run processes normally in the foreground or append `&` to run them asynchronously in the background.
- **Job Tracking (`activities`):** A custom builtin that lists all processes spawned by the shell, displaying their Process Group ID (PGID), command name, and current state (Running, Stopped, or Done).
- **Process Resumption (`fg` / `bg`):** Bring a stopped or background job into the foreground (`fg <job_id>`), or resume a stopped job in the background (`bg <job_id>`).
- **Signal Sending (`ping`):** Send specific signals directly to any process using its PID (`ping <pid> <signal_number>`).
- **Robust Signal Handling:** Safely intercepts `SIGINT` (Ctrl+C) and `SIGTSTP` (Ctrl+Z) to safely interrupt or suspend the foreground process stream without killing the shell itself. Listens asynchronously for `SIGCHLD` to gracefully reap dying background processes and update job states.

### 2. Advanced I/O & Pipelining
- **I/O Redirection:** Natively supports input redirection (`<`), output truncation (`>`), and output appending (`>>`).
- **N-Way Pipelining (`|`):** Supports arbitrarily long chains of piped commands (e.g., `cmd1 | cmd2 | cmd3`), managing all file descriptors and routing stdout to stdin sequentially across forked child processes. Processes in a pipeline are neatly assigned to a single Process Group.

### 3. Custom Built-In Commands
While the shell seamlessly executes standard system binaries (like `ls`, `cat`, `grep`) via `execvp`, it provides several custom built-ins for an enhanced experience:
- **`hop`:** Navigates the filesystem (similar to `cd`), maintaining a track record of previous and home directories.
- **`reveal`:** A custom implementation for listing directory contents and file metadata.
- **`log`:** A persistent history manager that tracks the last 15 valid commands across sessions. 
  - `log` - Prints the history.
  - `log purge` - Clears the history.
  - `log execute <index>` - Quickly re-runs a command from history.

## Tech Stack & Architecture
- **Language:** C (C99 standard)
- **APIs Used:** POSIX (`fork`, `execvp`, `pipe`, `dup2`, `signal`, `setpgid`, `tcsetpgrp`, `waitpid`)
- **Architecture:** Tokenizer -> Classifier -> Recursive Descent Parser -> Executor.

## Compilation & Usage

Use the provided `Makefile` to compile the shell:

```bash
# Compile the shell binary
make

# Run the shell
./shell.out
```

To clean up build artifacts:
```bash
make clean
```
