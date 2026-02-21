# CLAUDE.md - ObTools::Script Library

## Overview

`ObTools::Script` is an XML-based scripting language for composable test/workflow actions with parallel execution, timing, and variable scoping. Lives under `namespace ObTools::Script`.

**Header:** `ot-script.h`
**Dependencies:** `ot-xml`, `ot-init`, `ot-misc`, `ot-time`, `ot-log`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Context` | Variable scope (wraps `Misc::PropertyList`) |
| `Action` | Abstract base with start/tick/stop lifecycle |
| `SequenceAction` | Execute sub-actions sequentially |
| `RepeatAction` | Loop N times |
| `ParallelAction` | Execute concurrently (race or wait-all) |
| `ReplicatedAction` | Run N copies with spread timing |
| `Language` | Action registry and factory |
| `Script` | Top-level script container |

## Action Lifecycle

```cpp
virtual void start(Context& con) = 0;
virtual Action::Result tick(Context& con) = 0;  // COMPLETE, CONTINUE, ABORT
virtual void stop(Context& con) = 0;
```

## Built-in Actions

`BaseLanguage` registers: `sequence`, `repeat`, `group`, `race`, `replicate`, `delay`, `log`, `set`, `random`, `scope`, `thread`.
