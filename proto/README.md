# Proto

Shared protobuf definitions used between the SQLite collector and downstream services.

The schema is intentionally small:

- event IDs and device IDs are numeric
- event typing is explicit through enums
- payloads avoid strings and use integers or enums where possible
- the envelope stays stable while payloads vary through `oneof`
