# CLAUDE.md - ObTools::Init Library

## Overview

`ObTools::Init` provides automatic initialization sequencing and a factory/registry pattern for plugin-style object creation. Lives under `namespace ObTools::Init`.

**Header:** `ot-init.h`
**Dependencies:** `ot-xml`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Action` | Abstract initialisation action with rank |
| `Sequence` | Static sequencer — runs actions in rank order |
| `AutoAction` | Auto-registers itself on construction |
| `Factory<SUPER, CP>` | Abstract factory for object creation |
| `NewFactory<SUPER, SUB, CP>` | Factory using `new SUB(cp)` |
| `Registry<SUPER, CP, KEY>` | Named factory registry |
| `AutoRegister<SUPER, SUB, CP, KEY>` | Auto-registers factory in registry |

## Sequence

```cpp
static void Sequence::add(Action& a);
static void Sequence::run();              // runs all in rank order
```

## Action

```cpp
Action(int rank = 0);                     // 0 = default, 1+ = dependent
virtual void initialise() = 0;
```

## Factory / Registry

```cpp
template<typename SUPER, typename CP = const XML::Element&>
class Factory { virtual SUPER *create(CP cp) = 0; };

template<typename SUPER, typename CP, typename KEY = string>
class Registry {
  void add(const KEY& name, Factory<SUPER, CP>& f);
  SUPER *create(const KEY& name, CP cp);  // null if not found
};
```

## AutoRegister

```cpp
AutoRegister(Registry<SUPER, CP, KEY>& reg, const KEY& name);
// Auto-registers factory during Sequence::run()
```
