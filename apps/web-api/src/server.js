const http = require("http");

const port = Number(process.env.PORT || 3000);

function getHealth(res) {
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify({status: "ok"}));
}

function getDefault(res) {
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify({ message: "api: skeleton entrypoint" }));
}

const server = http.createServer((req, res) => {
    if (req.method === "GET" && req.url === "/health") {
        getHealth(res);
        return;
    }

    getDefault(res)
});

server.listen(port, () => {
    console.log(`api listening on port http://localhost:${port}`);
});
