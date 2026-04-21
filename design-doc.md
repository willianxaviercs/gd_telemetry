Create a monorepo skeleton for a distributed system with the following architecture:

SYSTEM OVERVIEW:
- Multiple devices generate data locally (SQLite)
- A collector service reads from SQLite and sends events to Redis Streams using Protobuf
- A consumer service reads from Redis and writes to Postgres
- A REST API reads from Postgres
- A simulator service generates fake data into SQLite

REQUIREMENTS:

1. REPO STRUCTURE

Create the following directories:

/proto        -> shared protobuf definitions
/collector    -> C++ service (SQLite -> Redis)
/consumer     -> C++ or C# service (Redis -> Postgres)
/simulator    -> process that writes events into SQLite
/api          -> REST API (simple, minimal)
/infra        -> docker-compose and infrastructure config

2. CONSTRAINTS

- Keep everything minimal and explicit
- DO NOT add unnecessary abstractions, frameworks, or patterns
- DO NOT implement business logic yet
- ONLY create structure, basic entrypoints, and placeholders
- Each service must have a clear and simple main entry file
- Include README.md in each service explaining its role

3. INFRASTRUCTURE

Create a docker-compose.yml with:
- postgres
- redis (with streams enabled)

Include basic environment variables but keep it simple.

4. PROTOBUF

Create a minimal proto file in /proto with:
- DeviceEvent
- device_id
- event_id
- timestamp
- oneof payload (at least 2 example event types)

Do NOT overcomplicate the schema.

5. LANGUAGE CHOICES

- collector: C++ (very C-like)
- consumer: C++  (very C-like)
- simulator: any simple language (Python preferred)
- API: simple (Node.js)

6. OUTPUT FORMAT

- Show full folder structure
- Generate all files
- Keep code minimal but runnable (hello world level)

IMPORTANT:
Do NOT implement full logic.
Do NOT add authentication, frontend, or extra features.
Focus ONLY on a clean, understandable skeleton.
We will vendor C++ dependencies (hiredis, sqlite, postgres) so we dont use package manager.


