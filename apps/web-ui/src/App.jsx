import { useEffect, useState } from "react";

import { fetchDeviceEvents, fetchDevices, fetchHealth } from "./api.js";

const typeOptions = [
  { value: "", label: "All event types" },
  { value: "1", label: "Position" },
  { value: "2", label: "Health" },
  { value: "3", label: "Mission" },
];

function formatTimestamp(timestamp) {
  return new Date(timestamp).toLocaleString();
}

function summarizeEvent(event) {
  const { payloadKind, payload } = event;

  if (payloadKind === "positionSample") {
    return `${payload.latitudeDeg.toFixed(4)}, ${payload.longitudeDeg.toFixed(4)} | alt ${payload.altitudeM}m | ${payload.speedMps}m/s`;
  }

  if (payloadKind === "healthSample") {
    return `battery ${payload.batteryPct}% | link ${payload.linkQualityPct}% | gps ${payload.gpsFixType}`;
  }

  if (payloadKind === "missionUpdate") {
    return `${payload.state} | ${payload.reason}`;
  }

  return "unknown payload";
}

function App() {
  const [health, setHealth] = useState({ status: "loading", db: "loading" });
  const [devices, setDevices] = useState([]);
  const [selectedDeviceId, setSelectedDeviceId] = useState(null);
  const [selectedType, setSelectedType] = useState("");
  const [events, setEvents] = useState([]);
  const [selectedEventId, setSelectedEventId] = useState(null);
  const [devicesError, setDevicesError] = useState("");
  const [eventsError, setEventsError] = useState("");
  const [isLoadingEvents, setIsLoadingEvents] = useState(false);

  useEffect(() => {
    let cancelled = false;

    async function loadHealth() {
      try {
        const nextHealth = await fetchHealth();
        if (!cancelled) {
          setHealth(nextHealth);
        }
      }
      catch {
        if (!cancelled) {
          setHealth({ status: "error", db: "unavailable" });
        }
      }
    }

    async function loadDevices() {
      try {
        const nextDevices = await fetchDevices();
        if (cancelled) {
          return;
        }

        setDevices(nextDevices);
        setSelectedDeviceId((current) => current ?? nextDevices[0]?.id ?? null);
      }
      catch (error) {
        if (!cancelled) {
          setDevicesError(error.message);
        }
      }
    }

    loadHealth();
    loadDevices();

    return () => {
      cancelled = true;
    };
  }, []);

  useEffect(() => {
    if (selectedDeviceId === null) {
      setEvents([]);
      setSelectedEventId(null);
      return;
    }

    let cancelled = false;

    async function loadEvents() {
      setIsLoadingEvents(true);
      setEventsError("");

      try {
        const nextEvents = await fetchDeviceEvents(selectedDeviceId, selectedType);
        if (cancelled) {
          return;
        }

        setEvents(nextEvents);
        setSelectedEventId(nextEvents[0]?.id ?? null);
      }
      catch (error) {
        if (!cancelled) {
          setEvents([]);
          setSelectedEventId(null);
          setEventsError(error.message);
        }
      }
      finally {
        if (!cancelled) {
          setIsLoadingEvents(false);
        }
      }
    }

    loadEvents();

    return () => {
      cancelled = true;
    };
  }, [selectedDeviceId, selectedType]);

  const selectedEvent = events.find((event) => event.id === selectedEventId) || null;

  return (
    <main className="app-shell">
      <header className="topbar">
        <div>
          <p className="eyebrow">Telemetry</p>
          <h1>Edge Event Console</h1>
        </div>
        <div className={`health-pill is-${health.status}`}>
          <span>API {health.status}</span>
          <span>DB {health.db}</span>
        </div>
      </header>

      <section className="workspace">
        <aside className="panel panel-devices">
          <div className="panel-header">
            <h2>Devices</h2>
            <span>{devices.length}</span>
          </div>

          {devicesError ? <p className="panel-error">{devicesError}</p> : null}

          <div className="device-list">
            {devices.map((device) => (
              <button
                key={device.id}
                className={device.id === selectedDeviceId ? "device-card is-active" : "device-card"}
                onClick={() => setSelectedDeviceId(device.id)}
                type="button"
              >
                <span className="device-label">Device </span>
                <strong>{device.id}</strong>
              </button>
            ))}
          </div>
        </aside>

        <section className="panel panel-events">
          <div className="panel-header">
            <h2>Events</h2>
            <label className="type-filter">
              <span>Type</span>
              <select value={selectedType} onChange={(event) => setSelectedType(event.target.value)}>
                {typeOptions.map((option) => (
                  <option key={option.value || "all"} value={option.value}>
                    {option.label}
                  </option>
                ))}
              </select>
            </label>
          </div>

          {eventsError ? <p className="panel-error">{eventsError}</p> : null}
          {isLoadingEvents ? <p className="panel-message">Loading events…</p> : null}
          {!isLoadingEvents && !eventsError && events.length === 0 ? (
            <p className="panel-message">No events found for this device.</p>
          ) : null}

          <div className="event-list">
            {events.map((event) => (
              <button
                key={event.id}
                className={event.id === selectedEventId ? "event-row is-active" : "event-row"}
                onClick={() => setSelectedEventId(event.id)}
                type="button"
              >
                <div className="event-meta">
                  <span className="event-type">{event.typeName}</span>
                  <span className="event-time">{formatTimestamp(event.timestamp)}</span>
                </div>
                <strong className="event-summary">{summarizeEvent(event)}</strong>
                <span className="event-kind">{event.payloadKind}</span>
              </button>
            ))}
          </div>
        </section>

        <aside className="panel panel-detail">
          <div className="panel-header">
            <h2>Event Detail</h2>
            {selectedEvent ? <span>#{selectedEvent.id}</span> : null}
          </div>

          {selectedEvent ? (
            <>
              <dl className="detail-grid">
                <div>
                  <dt>Device</dt>
                  <dd>{selectedEvent.deviceId}</dd>
                </div>
                <div>
                  <dt>Timestamp</dt>
                  <dd>{formatTimestamp(selectedEvent.timestamp)}</dd>
                </div>
                <div>
                  <dt>Type</dt>
                  <dd>{selectedEvent.typeName}</dd>
                </div>
                <div>
                  <dt>Payload</dt>
                  <dd>{selectedEvent.payloadKind}</dd>
                </div>
              </dl>

              <pre className="payload-view">
                <code>{JSON.stringify(selectedEvent.payload, null, 2)}</code>
              </pre>
            </>
          ) : (
            <p className="panel-message">Select an event to inspect its decoded payload.</p>
          )}
        </aside>
      </section>
    </main>
  );
}

export default App;
