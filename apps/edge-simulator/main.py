import argparse
import random
import sqlite3
import time
from dataclasses import dataclass
from pathlib import Path

from gen import payload_pb as payload

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
    mission_state: int = payload.MissionState.IDLE
    tick: int = 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Write synthetic drone events into SQLite.")
    parser.add_argument("--device-id", type=int, required=True)
    parser.add_argument("--interval-seconds", type=float, required=True)
    parser.add_argument("--db-path", type=Path, required=True)

    return parser.parse_args()

def build_position_payload(*, state: DroneState) -> bytes:
    result = payload.Payload()
    result.version               = PAYLOAD_VERSION
    result.position_sample.latitude_deg  = state.lat_deg
    result.position_sample.longitude_deg = state.lon_deg
    result.position_sample.altitude_m    = state.altitude_m
    result.position_sample.heading_deg   = state.heading_deg
    result.position_sample.speed_mps     = state.speed_mps

    return result.SerializeToString()


def build_health_payload(*, state: DroneState) -> bytes:
    result = payload.Payload()
    result.version                        = PAYLOAD_VERSION
    result.health_sample.battery_pct      = state.battery_pct
    result.health_sample.link_quality_pct = state.link_quality_pct
    result.health_sample.gps_fix_type     = state.gps_fix_type

    return result.SerializeToString()


def build_mission_payload(*, state: DroneState, reason: int) -> bytes:
    result = payload.Payload()
    result.version               = PAYLOAD_VERSION
    result.mission_update.state  = state.mission_state
    result.mission_update.reason = reason

    return result.SerializeToString()


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
        conn: sqlite3.Connection, *,
        device_id: int, timestamp: int, type: int, payload: bytes
        ) -> int:
    conn.execute(
        """
        INSERT INTO device_events
        (device_id, timestamp, type, payload)
        VALUES (?, ?, ?, ?)
        """,
        (device_id, timestamp, type, payload),
    )

def now_ms() -> int:
    return int(time.time() * 1000)

# TODO(wxr) - We need to start generating EVENT.TYPE
def main() -> None:
    args = parse_args()

    if not args.db_path.exists():
        raise SystemExit(f"missing sqlite database: {args.db_path}")

    conn= sqlite3.connect(args.db_path)
    conn.execute("PRAGMA journal_mode = WAL")

    print(f"simulator: device_id={args.device_id} db_path={args.db_path}")
    state = DroneState()
    state.mission_state = payload.MissionState.TAKEOFF

    ## first report at startup is a mission report
    startup_blob = build_mission_payload(
        state=state,
        reason=payload.MissionReason.STARTUP,
    )
    insert_event(conn, device_id=args.device_id, timestamp=now_ms(), type=0, payload=startup_blob)
    conn.commit()

    state.mission_state = payload.MissionState.PATROL

    try:
        while True:
            evolve_state(state)

            ## position report every tick
            position_blob = build_position_payload(state=state)
            insert_event(conn, device_id=args.device_id, timestamp=now_ms(), type=0, payload=position_blob)

            ## health report every 5 ticks
            if state.tick % 5 == 0:
                health_blob = build_health_payload(state=state)
                insert_event(conn, device_id=args.device_id, timestamp=now_ms(), type=0, payload=health_blob)

            #  mission report when battery is low
            if state.battery_pct <= 25 and state.mission_state != paylod.MissionState.RETURN_TO_HOME:
                state.mission_state = payload.MissionState.RETURN_TO_HOME
                mission_blob = build_mission_payload(state=state, reason=de.MissionReason.LOW_BATTERY)
                row_id = insert_event(conn, device_id=args.device_id, timestamp=now_ms(), type=0, payload=mission_blob)

            conn.commit()
            time.sleep(args.interval_seconds)

    except KeyboardInterrupt:
        print("simulator: stopping")
    finally:
        conn.close()


if __name__ == "__main__":
    main()

