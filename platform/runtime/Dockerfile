FROM alpine:3.20

WORKDIR /app

RUN apk add --no-cache python3 sqlite

COPY build/edge-daemon/edge-daemon ./daemon
COPY apps/edge-simulator/main.py ./simulator.py
COPY libs/schemas/sqlite/device_events.sql /tmp/schema.sql
COPY platform/runtime/tools/start.sh .

RUN sqlite3 ./device.db ".read /tmp/schema.sql"

CMD ["sh", "./start.sh"]

