# ObTools::CPPT

C++ template processor for code generation — embeds C++ code and expressions within template text using configurable delimiters.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-cppt.h"
using namespace ObTools;

CPPT::Tags tags;
tags.start_code = "<?";
tags.end_code = "?>";
tags.start_expr = "<?=";
tags.end_expr = "?>";
tags.start_comment = "<?#";
tags.end_comment = "?>";

istringstream input("Hello <?= name ?>! Today is <?= day ?>.");
ostringstream output;

CPPT::Processor proc(input, output, tags, "cout");
proc.process();

// output now contains C++ code like:
// cout << "Hello " << name << "! Today is " << day << ".";
```

### Template Syntax

```
Plain text           →  cout << "Plain text" << endl;
<? code; ?>          →  code;
<?= expr ?>          →  cout << expr;
<?# comment ?>       →  (removed)
```

### Example Template

```
<? for (int i = 0; i < n; i++) { ?>
  Item <?= i ?>: <?= items[i] ?>
<? } ?>
```

## Build

```
NAME    = ot-cppt
TYPE    = lib
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
