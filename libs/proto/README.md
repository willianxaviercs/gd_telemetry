# Proto

Shared protobuf definitions used between the edge daemon and downstream services.

When the protobuf C++ runtime and `protoc` are both available, the repo-level CMake build generates shared C++ sources once under `build/generated/proto/` so multiple C++ applications can use the same generated code.

The schema is intentionally small:

- event IDs and device IDs are numeric
- event typing is explicit through enums
- payloads avoid strings and use integers or enums where possible
- the envelope stays stable while payloads vary through `oneof`
