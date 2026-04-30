const http = require("http");
const path = require("path");
const protobuf = require("protobufjs");
const { Pool } = require("pg");

const port = Number(process.env.PORT || 3000);
const fixedEventLimit = 200;
const allowedOrigin = process.env.CORS_ALLOW_ORIGIN || "http://localhost:5173";
const mockDeviceIds = parseMockDeviceIds(process.env.MOCK_DEVICE_IDS);

const pool = new Pool({
    host: process.env.DB_HOST || "127.0.0.1",
    port: Number(process.env.DB_PORT || 5432),
    database: process.env.DB_NAME || "gundam",
    user: process.env.DB_USER || "gundam",
    password: process.env.DB_PASSWORD || "gundam",
});

const protoPath = path.join(__dirname, "../../../libs/proto/payload.proto");
const protoRoot = protobuf.loadSync(protoPath);
const payloadType = protoRoot.lookupType("proto.v1.Payload");
const eventTypeEnum = protoRoot.lookupEnum("proto.v1.EventType");

function parseMockDeviceIds(input) {
    if (!input) {
        return [0, 1, 2, 3, 4, 5];
    }

    return input
        .split(",")
        .map((value) => Number(value.trim()))
        .filter((value) => Number.isInteger(value) && value > 0);
}

function buildCorsHeaders(req) {
    const origin = req.headers.origin;
    if (origin && origin === allowedOrigin) {
        return {
            "Access-Control-Allow-Origin": origin,
            "Access-Control-Allow-Methods": "GET, OPTIONS",
            "Access-Control-Allow-Headers": "Content-Type",
            Vary: "Origin",
        };
    }

    return {};
}

function writeJson(req, res, statusCode, payload) {
    res.writeHead(statusCode, {
        "Content-Type": "application/json",
        ...buildCorsHeaders(req),
    });
    res.end(JSON.stringify(payload));
}

function writeError(req, res, statusCode, code, message) {
    writeJson(req, res, statusCode, {
        error: {
            code,
            message,
        },
    });
}

function parseDeviceId(pathname) {
    const match = pathname.match(/^\/devices\/(\d+)\/events$/);

    if (!match) {
        return null;
    }

    return Number(match[1]);
}

function parseOptionalType(rawType) {
    if (rawType === null) {
        return null;
    }

    const parsedType = Number(rawType);
    if (!Number.isInteger(parsedType) || parsedType < 0) {
        throw new Error("query param 'type' must be a non-negative integer");
    }

    return parsedType;
}

function decodePayload(buffer) {
    const message = payloadType.decode(buffer);
    const object = payloadType.toObject(message, {
        longs: Number,
        enums: String,
        oneofs: true,
    });
    const payloadKind = object.sUnion;

    if (!payloadKind || !(payloadKind in object)) {
        throw new Error("decoded payload does not contain a supported event body");
    }

    return {
        payloadKind,
        payload: object[payloadKind],
    };
}

function getEventTypeName(type) {
    return eventTypeEnum.valuesById[type] || "EVENT_TYPE_UNSPECIFIED";
}

async function queryDeviceEvents(deviceId, eventType) {
    const params = [deviceId];
    let query = `
        SELECT id, device_id, timestamp, type, payload
        FROM device_events
        WHERE device_id = $1
    `;

    if (eventType !== null) {
        params.push(eventType);
        query += " AND type = $2";
    }

    params.push(fixedEventLimit);
    query += `
        ORDER BY timestamp DESC, id DESC
        LIMIT $${params.length}
    `;

    const result = await pool.query(query, params);

    return result.rows.map((row) => {
        const type = Number(row.type);
        const { payloadKind, payload } = decodePayload(row.payload);

        // TODO: validate that row.type matches the protobuf oneof payload variant.
        return {
            id: Number(row.id),
            deviceId: Number(row.device_id),
            timestamp: Number(row.timestamp),
            type,
            typeName: getEventTypeName(type),
            payloadKind,
            payload,
        };
    });
}

async function getHealth(req, res) {
    try {
        await pool.query("SELECT 1");
        writeJson(req, res, 200, {
            status: "ok",
            db: "ok",
        });
    }
    catch (error) {
        console.error("health check failed", error);
        writeJson(req, res, 503, {
            status: "error",
            db: "unavailable",
        });
    }
}

function getDevices(req, res) {
    writeJson(req, res, 200, {
        devices: mockDeviceIds.map((id) => ({ id })),
    });
}

async function getDeviceEvents(req, res, pathname, searchParams) {
    const deviceId = parseDeviceId(pathname);
    if (deviceId === null) {
        writeError(req, res, 404, "not_found", "route not found");
        return;
    }

    if (!mockDeviceIds.includes(deviceId)) {
        writeError(req, res, 404, "device_not_found", `unknown device '${deviceId}'`);
        return;
    }

    let eventType = null;
    try {
        eventType = parseOptionalType(searchParams.get("type"));
    }
    catch (error) {
        writeError(req, res, 400, "invalid_type_filter", error.message);
        return;
    }

    try {
        const events = await queryDeviceEvents(deviceId, eventType);
        writeJson(req, res, 200, {
            deviceId,
            events,
        });
    }
    catch (error) {
        console.error("failed to fetch device events", error);
        writeError(req, res, 500, "device_events_query_failed", "failed to fetch device events");
    }
}

const server = http.createServer(async (req, res) => {
    const url = new URL(req.url, `http://${req.headers.host || "localhost"}`);
    const { pathname, searchParams } = url;

    if (req.method === "OPTIONS") {
        res.writeHead(204, buildCorsHeaders(req));
        res.end();
        return;
    }

    if (req.method === "GET" && pathname === "/health") {
        await getHealth(req, res);
        return;
    }

    if (req.method === "GET" && pathname === "/devices") {
        getDevices(req, res);
        return;
    }

    if (req.method === "GET" && /^\/devices\/\d+\/events$/.test(pathname)) {
        await getDeviceEvents(req, res, pathname, searchParams);
        return;
    }

    writeError(req, res, 404, "not_found", "route not found");
});

server.listen(port, () => {
    console.log(`api listening on http://localhost:${port}`);
});
