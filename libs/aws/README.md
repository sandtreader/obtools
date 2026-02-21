# ObTools::AWS

Amazon Web Services S3 client with AWS Signature Version 4 authentication.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-aws.h"
using namespace ObTools;

SSL_OpenSSL::Context ctx;
ctx.set_default_verify_paths();

AWS::S3Client s3("ACCESS_KEY", "SECRET_KEY", &ctx);
s3.set_region("us-east-1");

// Upload
s3.create_object("my-bucket", "key.txt", "content", "text/plain");

// Download
string data;
s3.get_object("my-bucket", "key.txt", data);

// List
list<string> keys;
s3.list_bucket("my-bucket", "prefix/", keys);

// Delete
s3.delete_object("my-bucket", "key.txt");
```

## Build

```
NAME      = ot-aws
TYPE      = lib
DEPENDS   = ot-crypto ot-web ot-misc
PLATFORMS = posix
```

## License

Copyright (c) 2017 Paul Clark. MIT License.
