# CLAUDE.md - ObTools::Access Library

## Overview

`ObTools::Access` provides XML-configured access control (ACL) with groups, users, IP addresses, and resource matching. Lives under `namespace ObTools::Access`.

**Header:** `ot-access.h`
**Dependencies:** `ot-xml`, `ot-ssl`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Checker` | Main ACL checker (groups + resources) |
| `Resource` | Resource with allow/deny rules |
| `Rule` | Individual access rule (AND of conditions) |
| `Group` | User group with glob pattern matching |

## Checker

```cpp
Checker(const XML::Element& config, const string& ns = "");
bool check(const string& resource, Net::IPAddress address, const string& user);
bool check(const string& resource, const SSL::ClientDetails& client);
void dump(ostream& sout) const;
```

## Resource

```cpp
Resource(const XML::Element& resource_e, map<string, Group *>& groups,
         const string& ns = "");
bool check(const string& resource, Net::IPAddress address,
           const string& user, bool& result_p);
```

## Rule

```cpp
Rule(Group *group, const string& user, Net::MaskedAddress address);
Rule(const XML::Element& r_e, map<string, Group *>& groups);
bool matches(Net::IPAddress attempted_address, const string& attempted_user);
```

## Group

```cpp
Group(const XML::Element& group_e, const string& ns = "");
string get_id();
bool contains(const string& user);  // case-insensitive glob
```

## XML Configuration Format

```xml
<access>
  <group id="admins">
    <user>alice</user>
    <user>bob*</user>
  </group>
  <resource name="/admin/*">
    <allow group="admins"/>
    <deny address="0.0.0.0/0"/>
  </resource>
</access>
```
