# ObTools::Script

XML-based scripting language for composable test/workflow actions with parallel execution, timing, and variable scoping.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-script.h"
using namespace ObTools;

Script::BaseLanguage lang;
// Register custom actions...
Script::Script script(lang);
script.read_from(xml_config);
script.run();
```

## Language Overview

A script is an XML document whose root element is `<script>`. Its children are
*actions*, executed in document order (i.e. the script is itself a sequence).
Every action has a lifecycle of `start` → repeated `tick` → `stop`; actions that
take time (e.g. `<delay>`) stay active over multiple ticks, while simple ones
(e.g. `<log>`, `<set>`) complete immediately.

```xml
<!-- Simplest possible script -->
<script>
  <log>Hello, world</log>
</script>
```

### Variables and interpolation

Variables live in a `Context`. Text content (and where noted, attributes) is
interpolated, replacing `$name` with the variable's value. Variables are set
with `<set>`, and some actions inject their own (e.g. `repeat` sets `$index`,
`replicate` sets `$copy`).

`BaseLanguage` registers all the constructs below. Applications may register
their own additional actions via `Language::register_action()`.

## Language Constructs

### `<log>` — write a log message

Writes its (interpolated) content to the ObTools logger.

| Attribute | Default | Description |
|-----------|---------|-------------|
| `level`   | `summary` | Numeric log level |

```xml
<log>Hello, world</log>
<log level="2">Iteration $index</log>
```

### `<set>` — assign a variable

Sets the variable named by `var` to its (interpolated) content. References to
existing variables can be used to build new values.

| Attribute | Description |
|-----------|-------------|
| `var`     | Name of the variable to set (required) |

```xml
<set var="foo">FOO!</set>
<set var="bar">$foo BAR!</set>   <!-- bar becomes "FOO! BAR!" -->
<log>$bar</log>
```

### `<sequence>` — run actions in order

Executes its child actions one after another, each completing before the next
begins. The top-level `<script>` is itself a sequence, so an explicit
`<sequence>` is only needed for nesting inside another construct.

```xml
<sequence>
  <log>First</log>
  <delay time="1"/>
  <log>Second</log>
</sequence>
```

### `<repeat>` — loop

Repeats its child sequence. If `times` is given it loops that many times;
otherwise it loops forever. The current zero-based iteration number is available
as `$index`.

| Attribute | Default | Description |
|-----------|---------|-------------|
| `times`   | forever | Number of iterations |

```xml
<repeat times="10">
  <log>Iteration $index</log>
</repeat>
```

### `<group>` — run actions in parallel (wait for all)

Starts all its children simultaneously and continues until the **last** one
finishes. Each branch runs as an interleaved (cooperative) action, not a real
OS thread.

```xml
<group>
  <repeat times="5">
    <log>Thread 1</log>
  </repeat>
  <repeat times="10">
    <log>Thread 2</log>
  </repeat>
</group>
```

### `<race>` — run actions in parallel (stop at first)

Like `<group>`, but the whole construct stops as soon as the **first** child
finishes; the others are then stopped.

```xml
<race>
  <repeat times="5">
    <log>Thread 1</log>
  </repeat>
  <repeat times="10">
    <log>Thread 2</log>
  </repeat>
</race>
```

### `<replicate>` — run N copies of a sequence

Runs `copies` independent copies of its child sequence. New copies are started
`spread` apart in time, so load can be ramped up gradually. Each copy sees its
zero-based copy number as `$copy`. The construct finishes when all copies have
started and completed.

| Attribute | Default | Description |
|-----------|---------|-------------|
| `copies`  | `1`     | Number of copies to run |
| `spread`  | `0`     | Time between copy starts (duration, e.g. `1 sec`) |

```xml
<replicate copies="5" spread="1 sec">
  <log>Copy $copy: One</log>
  <log>Copy $copy: Two</log>
  <log>Copy $copy: Three</log>
</replicate>
```

### `<delay>` — wait

Pauses for the given `time`. With `random="yes"` the actual delay is randomised
between zero and `time`.

| Attribute | Default | Description |
|-----------|---------|-------------|
| `time`    | `0`     | Duration to wait (e.g. `2`, `5 sec`) |
| `random`  | `no`    | If set, randomise the delay up to `time` |

```xml
<delay time="2"/>
<delay time="5" random="yes"/>
```

### `<random>` — conditional execution by probability

At creation, decides with the given `probability` (0–1) whether to run its child
sequence at all. Useful inside a `<repeat>` to trigger occasional events.

| Attribute     | Default | Description |
|---------------|---------|-------------|
| `probability` | `0`     | Chance (0–1) of running the contents |

```xml
<repeat>
  <random probability="0.1">
    <log>Randomness!</log>
  </random>
</repeat>
```

### `<scope>` — introduce a new variable scope

Runs its child sequence in a copy of the current context. Variables from the
outer scope are visible inside, but changes made inside are **not** propagated
back out (copy-in, not copy-out).

```xml
<group>
  <set var="foo">FOO!</set>
  <log>Outer scope 1, 'foo' = $foo</log>   <!-- FOO! -->
  <scope>
    <group>
      <set var="foo">BAR!</set>
      <log>Inner scope, 'foo' = $foo</log>  <!-- BAR! -->
    </group>
  </scope>
  <log>Outer scope 2, 'foo' = $foo</log>    <!-- still FOO! -->
</group>
```

### `<thread>` — run in a real OS thread

Runs its child sequence in a separate processor thread, with its own context.
Use this for genuinely blocking sub-actions (e.g. network connections) that
would otherwise stall the cooperative tick loop. `sleep` sets the pause (in
microseconds) between ticks of the contained sequence.

| Attribute | Default | Description |
|-----------|---------|-------------|
| `sleep`   | `10000` | Microseconds between child ticks |

```xml
<group>
  <thread sleep="1000000">
    <repeat times="10">
      <log>Thread 1 iteration $index</log>
    </repeat>
    <log>Thread 1 finished</log>
  </thread>

  <thread sleep="100000">
    <repeat times="100">
      <log>Thread 2 iteration $index</log>
    </repeat>
    <log>Thread 2 finished</log>
  </thread>
</group>
<log>All done!</log>
```

> **`<group>`/`<race>` vs `<thread>`:** `<group>` and `<race>` interleave their
> branches cooperatively within a single thread on each tick, so a blocking
> action in one branch stalls the others. `<thread>` runs its contents on a real
> OS thread, so it keeps going independently — at the cost of needing its own
> context.

## Build

```
NAME    = ot-script
TYPE    = lib
DEPENDS = ot-xml ot-init ot-misc ot-time ot-log
```

## Examples

Runnable example scripts for every construct live in [`tests/`](tests/).

## License

Copyright (c) 2012 Paul Clark. MIT License.
