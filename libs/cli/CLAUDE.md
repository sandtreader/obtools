# CLAUDE.md - ObTools::CLI Library

## Overview

`ObTools::CLI` provides a command-line interface framework with hierarchical command groups, interactive prompts, and telnet support. Lives under `namespace ObTools::CLI`.

**Header:** `ot-cli.h`
**Dependencies:** `ot-net`, `ot-text`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Handler` | Abstract command handler interface |
| `MemberHandler<T>` | Template binding member functions as handlers |
| `Command` | Single command with help and usage |
| `CommandGroup` | Hierarchical group of commands |
| `Registry` | Top-level command registry |
| `CommandLine` | Interactive command-line loop |

## Handler

```cpp
class Handler {
  virtual void handle(string args, istream& sin, ostream& sout) = 0;
};
```

## MemberHandler<T>

```cpp
MemberHandler(T& object, void (T::*func)(string, istream&, ostream&));
```

## Command

```cpp
Command(const string& word, Handler *handler,
        const string& help = "", const string& usage = "");
void handle(string args, istream& sin, ostream& sout);
void show_usage(ostream& sout);
```

## CommandGroup

```cpp
CommandGroup(const string& word, const string& help = "");
void add(Command *command);       // auto-nests by prefix
void handle(string args, istream& sin, ostream& sout);
void show_help(ostream& sout);
```

## Registry

```cpp
Registry();
void add(const string& prefix, const string& help = "");
void add(const string& word, Handler *handler,
         const string& help = "", const string& usage = "");
```

## CommandLine

```cpp
CommandLine(Registry& registry, istream& sin, ostream& sout,
            const string& prompt = ">");
void handle(string cmd);
bool readline(string& line);      // false on EOF
void run();                        // interactive loop
```
