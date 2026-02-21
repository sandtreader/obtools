# CLAUDE.md - ObTools::Lang Library

## Overview

`ObTools::Lang` provides ISO 639 language code enumeration and conversion for 500+ languages. Lives under `namespace ObTools::Lang`.

**Header:** `ot-lang.h`
**Dependencies:** none

## API

```cpp
enum class Language { english, french, german, chinese, ... };  // 500+ values

Language iso_639_to_lang(const string& code);      // 2 or 3 char -> enum
Language iso_639_1_to_lang(const string& code);    // 2-char -> enum
Language iso_639_2_to_lang(const string& code);    // 3-char -> enum
string lang_to_iso_639_1(Language lang);           // enum -> 2-char
string lang_to_iso_639_2(Language lang);           // enum -> 3-char
string lang_to_string(Language lang);              // enum -> "English"
ostream& operator<<(ostream&, Language);
```
