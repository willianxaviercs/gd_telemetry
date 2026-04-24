#!/bin/bash

set -e

: "${DEVICE_ID:?missing DEVICE_ID}"
: "${INTERVAL_SECONDS:?missing INTERVAL_SECONDS}"
: "${REDIS_HOST:?missing REDIS_HOST}"
: "${REDIS_PORT:?missing REDIS_PORT}"

echo "starting simulator"

python3 -u ./simulator.py \
    --device-id $DEVICE_ID \
    --interval-seconds $INTERVAL_SECONDS \
    --db-path ./device.db > simulator.log 2>&1 &

SIM_PID=$!

echo "starting daemon"

./daemon --device-db-path ./device.db \
         --poll-interval-seconds $INTERVAL_SECONDS \
         --redis-stream event_stream \
         --redis-host $REDIS_HOST \
         --redis-port $REDIS_PORT > daemon.log 2>&1 &

DAEMON_PID=$!

wait $SIM_PID $DAEMON_PID

