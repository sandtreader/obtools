# CLAUDE.md - ObTools::Netlink Library

## Overview

`ObTools::Netlink` provides a C++ wrapper for Linux Generic Netlink protocol, enabling type-safe netlink message construction and parsing. Lives under `namespace ObTools::Netlink`.

**Header:** `ot-netlink.h`
**Dependencies:** `ext-pkg-libnl-3.0`, `ext-pkg-libnl-genl-3.0`
**Platforms:** Linux only

## Key Classes

| Class | Purpose |
|-------|---------|
| `GenericNetlink` | Main netlink socket and protocol handler |
| `GenericRequest` | Base class for outgoing requests |
| `GenericResponse` | Incoming message parser with attribute extraction |

## GenericNetlink

```cpp
GenericNetlink(const string& family);    // resolves family name
bool valid() const;
int get_family() const;
const char* get_last_error();
bool send(GenericRequest& request);      // send and handle response
```

## GenericRequest (subclass for your commands)

```cpp
GenericRequest(const GenericNetlink& netlink, uint8_t command,
               uint8_t version, int flags = 0);

// Override these
virtual int callback(GenericResponse&);
virtual int get_attribute_count();
virtual struct nla_policy *get_policy();

// Set attributes
void set_string(int attr, const string& s);
void set_uint32(int attr, uint32_t u);
void set_uint16(int attr, uint16_t u);
void set_buffer(int attr, void *buff, ssize_t len);
bool begin_nest(int attr);
void end_nest();
```

## GenericResponse

```cpp
// Extract attributes (with or without custom attr vector)
uint16_t get_uint16(int attr);
uint32_t get_uint32(int attr);
string get_string(int attr);
bool get_data(int attr, void *buffer, ssize_t len);
bool get_nested_attrs(int attr, vector<struct nlattr *>& nested_attrs,
                      int attrs_max, struct nla_policy* policy);
int error();
```
