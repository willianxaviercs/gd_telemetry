const apiBaseUrl = (import.meta.env.VITE_API_BASE_URL || "http://localhost:3000").replace(/\/+$/, "");

async function readJson(response) {
  const contentType = response.headers.get("content-type") || "";
  const body = contentType.includes("application/json") ? await response.json() : null;

  if (!response.ok) {
    const message = body?.error?.message || `request failed with status ${response.status}`;
    throw new Error(message);
  }

  return body;
}

export async function fetchHealth() {
  const response = await fetch(`${apiBaseUrl}/health`);
  return readJson(response);
}

export async function fetchDevices() {
  const response = await fetch(`${apiBaseUrl}/devices`);
  const body = await readJson(response);
  return body.devices || [];
}

export async function fetchDeviceEvents(deviceId, eventType) {
  const url = new URL(`${apiBaseUrl}/devices/${deviceId}/events`);
  if (eventType !== "") {
    url.searchParams.set("type", eventType);
  }

  const response = await fetch(url);
  const body = await readJson(response);
  return body.events || [];
}
