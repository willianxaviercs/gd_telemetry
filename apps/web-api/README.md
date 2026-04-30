# API

Minimal Node.js REST API for edge-devices

- reading data from Postgres
- exposing simple HTTP endpoints

Getting device list:

```bash
curl -X GET http://localhost:3000/devices
```

Getting all events for a device:

```bash
curl -X GET http://localhost:3000/devices/:id/events
```

