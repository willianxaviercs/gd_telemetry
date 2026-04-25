# Proto

Shared protobuf definitions used by simulator/producer tooling and downstream services.

The C++ edge daemon currently validates and forwards protobuf payloads without linking
the protobuf C++ runtime.

The schema is intentionally small:

- event IDs and device IDs are numeric
- event typing is explicit through enums
- payloads avoid strings and use integers or enums where possible
- the envelope stays stable while payloads vary through `oneof`

## Regenerating Python protobuf sources

```bash
protoc \
  --proto_path=libs/proto \
  --python_out=apps/edge-simulator/gen \
  libs/proto/device_event.proto
```
