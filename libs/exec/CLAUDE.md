# CLAUDE.md - ObTools::Exec Library

## Overview

`ObTools::Exec` provides helpers for spawning and executing sub-processes with stdin/stdout/stderr handling. Lives under `namespace ObTools::Exec`.

**Header:** `ot-exec.h`
**Dependencies:** `ot-log`, `ot-text`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Command` | Executes a command with optional I/O capture |

## Command

```cpp
Command(const string& command_with_args);     // splits into argv
Command(const vector<string>& args);          // pre-split

bool execute(const string& input, string& output);  // stdin + capture stdout
bool execute(string& output);                        // capture stdout only
bool execute();                                      // fire and forget
```

## Implementation Notes

- Uses Unix fork/exec with piped stdin/stdout/stderr
- stderr output is logged via `Log::Error`
- Runs two capture threads for stdout (buffered) and stderr (logged)
- Empty environment passed to child process
- Returns false on Windows (unsupported)
