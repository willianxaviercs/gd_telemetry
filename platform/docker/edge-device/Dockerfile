FROM alpine:3.20

WORKDIR /app

RUN apk add --no-cache python3 py3-pip sqlite
RUN python3 -m pip install --break-system-packages --no-cache-dir "protobuf==6.33.4"
RUN mkdir ./gen

COPY build/edge-daemon/edge-daemon ./daemon
COPY apps/edge-simulator/main.py ./simulator.py
COPY apps/edge-simulator/gen/ ./gen
COPY libs/schemas/sqlite/device_events.sql /tmp/schema.sql
COPY platform/runtime/tools/start.sh .

RUN sqlite3 ./device.db ".read /tmp/schema.sql"

CMD ["sh", "./start.sh"]
