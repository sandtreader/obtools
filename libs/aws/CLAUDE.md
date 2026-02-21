# CLAUDE.md - ObTools::AWS Library

## Overview

`ObTools::AWS` provides an Amazon Web Services S3 client with AWS Signature V4 authentication. Lives under `namespace ObTools::AWS`.

**Header:** `ot-aws.h`
**Dependencies:** `ot-crypto`, `ot-web`, `ot-misc`
**Platforms:** posix

## Key Classes

| Class | Purpose |
|-------|---------|
| `Authenticator` | AWS request signing (HMAC-SHA256 Signature V4) |
| `S3Client` | S3 bucket and object operations |

## S3Client

```cpp
S3Client(const string& access_key, const string& secret_key, SSL::Context *ctx=nullptr);

void set_region(const string& region);
void set_user_agent(const string& ua);
void set_timeouts(int conn, int op);
void enable_persistence();
void enable_virtual_hosts();

string get_url(const string& bucket, const string& key="");

// Bucket operations
bool list_all_my_buckets(list<string>& buckets);
bool list_bucket(const string& bucket, const string& prefix, list<string>& keys);
bool create_bucket(const string& bucket);

// Object operations
bool create_object(const string& bucket, const string& key, const string& data, const string& content_type="");
bool get_object(const string& bucket, const string& key, string& data);
bool delete_object(const string& bucket, const string& key);
bool delete_multiple_objects(const string& bucket, const list<string>& keys);
bool delete_objects_with_prefix(const string& bucket, const string& prefix);
bool empty_bucket(const string& bucket);
```
