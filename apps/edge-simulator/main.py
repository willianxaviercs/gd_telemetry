import argparse
import random
import sqlite3
import time
from dataclasses import dataclass
from pathlib import Path

from gen import device_event_pb2 as de

PAYLOAD_VERSION = 1

@dataclass
class DroneState:
    lat_deg: float = -23.5505
    lon_deg: float = -46.6333
    altitude_m: float = 10.0
    heading_deg: float = 90.0
    speed_mps: float = 6.0
    battery_pct: int = 100
    link_quality_pct: int = 98
    gps_fix_type: int = 3
    mission_state: int = de.MissionState.IDLE
    tick: int = 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write synthetic drone events into SQLite.")
    parser.add_argument("--device-id", type=int, required=True)
    parser.add_argument("--interval-seconds", type=float, required=True)
    parser.add_argument("--db-path", type=Path, required=True)
    return parser.parse_args()

def build_position_event(*, event_id: int, device_id: int, timestamp_unix_ms: int, state: DroneState) -> bytes:
    event = de.DeviceEvent(
        event_id=event_id,
        device_id=device_id,
        timestamp_unix_ms=timestamp_unix_ms,
        type=de.EventType.POSITION_SAMPLE,
        payload_version=PAYLOAD_VERSION,
    )
    event.position_sample.latitude_deg = state.lat_deg
    event.position_sample.longitude_deg = state.lon_deg
    event.position_sample.altitude_m = state.altitude_m
    event.position_sample.heading_deg = state.heading_deg
    event.position_sample.speed_mps = state.speed_mps
    return event.SerializeToString()


def build_health_event(*, event_id: int, device_id: int, timestamp_unix_ms: int, state: DroneState) -> bytes:
    event = de.DeviceEvent(
        event_id=event_id,
        device_id=device_id,
        timestamp_unix_ms=timestamp_unix_ms,
        type=de.EventType.HEALTH_SAMPLE,
        payload_version=PAYLOAD_VERSION,
    )
    event.health_sample.battery_pct = state.battery_pct
    event.health_sample.link_quality_pct = state.link_quality_pct
    event.health_sample.gps_fix_type = state.gps_fix_type
    return event.SerializeToString()


def build_mission_event(
    *,
    event_id: int,
    device_id: int,
    timestamp_unix_ms: int,
    state: DroneState,
    reason: int,
) -> bytes:
    event = de.DeviceEvent(
        event_id=event_id,
        device_id=device_id,
        timestamp_unix_ms=timestamp_unix_ms,
        type=de.EventType.MISSION_UPDATE,
        payload_version=PAYLOAD_VERSION,
    )
    event.mission_update.state = state.mission_state
    event.mission_update.reason = reason
    return event.SerializeToString()


def evolve_state(state: DroneState) -> None:
    state.tick += 1
    state.heading_deg = (state.heading_deg + random.uniform(-10.0, 10.0)) % 360.0
    state.speed_mps = max(3.0, min(9.0, state.speed_mps + random.uniform(-0.5, 0.5)))

    heading_rad = state.heading_deg * 3.141592653589793 / 180.0
    lat_step = (state.speed_mps * 0.0000015) * (1.0 if heading_rad < 3.141592653589793 else -1.0)
    lon_step = (state.speed_mps * 0.0000015) * (1.0 if heading_rad < 1.5707963267948966 or heading_rad > 4.71238898038469 else -1.0)
    state.lat_deg += lat_step
    state.lon_deg += lon_step
    state.altitude_m = max(10.0, min(80.0, state.altitude_m + random.uniform(-1.2, 1.2)))

    if state.tick % 3 == 0:
        state.battery_pct = max(5, state.battery_pct - 1)
    state.link_quality_pct = max(70, min(100, state.link_quality_pct + random.randint(-2, 1)))


def insert_event(
    connection: sqlite3.Connection,
    *,
    event_blob: bytes,
) -> int:
    cursor = connection.execute(
        """
        INSERT INTO device_events (event_blob)
        VALUES (?)
        """,
        (event_blob,),
    )
    return cursor.lastrowid


def main() -> None:
    args = parse_args()

    if not args.db_path.exists():
        raise SystemExit(f"missing sqlite database: {args.db_path}")

    connection = sqlite3.connect(args.db_path)
    connection.execute("PRAGMA journal_mode = WAL")

    print(f"simulator: device_id={args.device_id} db_path={args.db_path}")
    state = DroneState()
    state.mission_state = de.MissionState.TAKEOFF
    next_event_id = 1

    startup_ts = int(time.time() * 1000)
    startup_blob = build_mission_event(
        event_id=next_event_id,
        device_id=args.device_id,
        timestamp_unix_ms=startup_ts,
        state=state,
        reason=de.MissionReason.STARTUP,
    )
    startup_id = insert_event(
        connection,
        event_blob=startup_blob,
    )
    next_event_id += 1
    connection.commit()
    print(f"simulator: inserted id={startup_id} type={int(de.EventType.MISSION_UPDATE)}")
    state.mission_state = de.MissionState.PATROL

    try:
        while True:
            evolve_state(state)
            now_ms = int(time.time() * 1000)

            position_blob = build_position_event(
                event_id=next_event_id,
                device_id=args.device_id,
                timestamp_unix_ms=now_ms,
                state=state,
            )
            row_id = insert_event(
                connection,
                event_blob=position_blob,
            )
            next_event_id += 1
            event_type = int(de.EventType.POSITION_SAMPLE)

            if state.tick % 5 == 0:
                health_blob = build_health_event(
                    event_id=next_event_id,
                    device_id=args.device_id,
                    timestamp_unix_ms=now_ms,
                    state=state,
                )
                row_id = insert_event(
                    connection,
                    event_blob=health_blob,
                )
                next_event_id += 1
                event_type = int(de.EventType.HEALTH_SAMPLE)

            if state.battery_pct == 25 and state.mission_state != de.MissionState.RETURN_TO_HOME:
                state.mission_state = de.MissionState.RETURN_TO_HOME
                mission_blob = build_mission_event(
                    event_id=next_event_id,
                    device_id=args.device_id,
                    timestamp_unix_ms=now_ms,
                    state=state,
                    reason=de.MissionReason.LOW_BATTERY,
                )
                row_id = insert_event(
                    connection,
                    event_blob=mission_blob,
                )
                next_event_id += 1
                event_type = int(de.EventType.MISSION_UPDATE)

            connection.commit()
            print(f"simulator: inserted id={row_id} type={event_type}")
            time.sleep(args.interval_seconds)
    except KeyboardInterrupt:
        print("simulator: stopping")
    finally:
        connection.close()


if __name__ == "__main__":
    main()
