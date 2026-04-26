FROM alpine:3.20

WORKDIR /app

RUN apk add --no-cache python3 py3-protobuf sqlite
RUN mkdir ./gen

COPY build/edge-daemon/edge-daemon ./daemon
COPY apps/edge-simulator/main.py ./simulator.py
COPY apps/edge-simulator/gen/device_event_pb2.py ./gen
COPY libs/schemas/sqlite/device_events.sql /tmp/schema.sql
COPY platform/runtime/tools/start.sh .

RUN sqlite3 ./device.db ".read /tmp/schema.sql"

CMD ["sh", "./start.sh"]
