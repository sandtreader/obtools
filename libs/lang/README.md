# ObTools::Lang

ISO 639 language code enumeration and conversion for 500+ languages including constructed, ancient, and regional variants.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-lang.h"
using namespace ObTools;

Lang::Language lang = Lang::iso_639_to_lang("en");
string name = Lang::lang_to_string(lang);           // "English"
string code2 = Lang::lang_to_iso_639_1(lang);       // "en"
string code3 = Lang::lang_to_iso_639_2(lang);       // "eng"
```

## Build

```
NAME = ot-lang
TYPE = lib
```

## License

Copyright (c) 2019 Paul Clark. MIT License.
