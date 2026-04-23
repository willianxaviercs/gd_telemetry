import argparse
import random
import sqlite3
import time
from pathlib import Path

EVENT_TYPE_TEMPERATURE_READING = 1
EVENT_TYPE_STATUS_UPDATE = 2

DEVICE_STATUS_ONLINE = 1
DEVICE_STATUS_OFFLINE = 2
DEVICE_STATUS_ERROR = 3


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write random device events into SQLite.")
    parser.add_argument("--device-id", type=int, required=True)
    parser.add_argument("--interval-seconds", type=float, required=True)
    return parser.parse_args()


def db_path_for_device(device_id: int) -> Path:
    repo_root = Path(__file__).resolve().parent.parent
    return repo_root / "runtime" / "devices" / str(device_id) / "device.db"


def insert_random_event(connection: sqlite3.Connection, device_id: int) -> tuple[int, int]:
    timestamp_unix_ms = int(time.time() * 1000)

    if random.choice((True, False)):
        temperature_celsius = round(random.uniform(18.0, 32.0), 2)
        cursor = connection.execute(
            """
            INSERT INTO device_events (
                device_id,
                timestamp_unix_ms,
                type,
                temperature_celsius,
                status
            ) VALUES (?, ?, ?, ?, NULL)
            """,
            (
                device_id,
                timestamp_unix_ms,
                EVENT_TYPE_TEMPERATURE_READING,
                temperature_celsius,
            ),
        )
        return cursor.lastrowid, EVENT_TYPE_TEMPERATURE_READING

    status = random.choice((DEVICE_STATUS_ONLINE, DEVICE_STATUS_OFFLINE, DEVICE_STATUS_ERROR))
    cursor = connection.execute(
        """
        INSERT INTO device_events (
            device_id,
            timestamp_unix_ms,
            type,
            temperature_celsius,
            status
        ) VALUES (?, ?, ?, NULL, ?)
        """,
        (
            device_id,
            timestamp_unix_ms,
            EVENT_TYPE_STATUS_UPDATE,
            status,
        ),
    )
    return cursor.lastrowid, EVENT_TYPE_STATUS_UPDATE


def main() -> None:
    args = parse_args()
    db_path = db_path_for_device(args.device_id)

    if not db_path.exists():
        raise SystemExit(f"missing sqlite database: {db_path}")

    connection = sqlite3.connect(db_path)
    connection.execute("PRAGMA journal_mode = WAL")

    print(f"simulator: device_id={args.device_id} db_path={db_path}")

    try:
        while True:
            row_id, event_type = insert_random_event(connection, args.device_id)
            connection.commit()
            print(f"simulator: inserted id={row_id} type={event_type}")
            time.sleep(args.interval_seconds)
    except KeyboardInterrupt:
        print("simulator: stopping")
    finally:
        connection.close()


if __name__ == "__main__":
    main()
