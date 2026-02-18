'use strict';

/**
 * HTTP bridge de desarrollo para probar PrinterBackendApi con Postman.
 *
 * Requiere que BACKEND_FACTORY apunte a un módulo que exporte:
 *   - una función (sync/async) que retorne instancia de PrinterBackendApi, o
 *   - directamente una instancia de PrinterBackendApi.
 *
 * Ejemplo:
 *   BACKEND_FACTORY=./local/create-printer-backend-api.js PORT=5050 node docs/examples/printer-backend-api-http-bridge.js
 */

const http = require('http');
const path = require('path');

function readJsonBody(req) {
  return new Promise((resolve, reject) => {
    let body = '';

    req.on('data', chunk => {
      body += chunk;
    });

    req.on('end', () => {
      if (!body) {
        resolve({});
        return;
      }

      try {
        resolve(JSON.parse(body));
      } catch (err) {
        reject(new Error('Invalid JSON body'));
      }
    });

    req.on('error', reject);
  });
}

function sendJson(res, statusCode, payload) {
  const body = JSON.stringify(payload);

  res.writeHead(statusCode, {
    'Content-Type': 'application/json',
    'Content-Length': Buffer.byteLength(body)
  });

  res.end(body);
}

async function loadBackend() {
  const factoryPath = process.env.BACKEND_FACTORY;

  if (!factoryPath) {
    throw new Error('Missing BACKEND_FACTORY env var');
  }

  const resolvedPath = path.resolve(process.cwd(), factoryPath);
  const factoryOrInstance = require(resolvedPath);

  if (typeof factoryOrInstance === 'function') {
    return factoryOrInstance();
  }

  return factoryOrInstance;
}

async function main() {
  const backendApi = await loadBackend();
  const latestFrameByUid = new Map();

  backendApi.on('camera-frame', ({ uid, encoding, frame }) => {
    const normalizedFrame = Buffer.isBuffer(frame) ? frame.toString('base64') : frame;

    latestFrameByUid.set(uid, {
      uid,
      encoding,
      frame: normalizedFrame,
      ts: new Date().toISOString()
    });
  });

  backendApi.on('state-changed', evt => {
    console.log('[state-changed]', evt);
  });

  const server = http.createServer(async (req, res) => {
    try {
      if (req.method === 'GET' && req.url === '/health') {
        sendJson(res, 200, { ok: true });
        return;
      }

      if (req.method === 'POST' && req.url === '/connectByIp') {
        const { ip } = await readJsonBody(req);
        const result = await backendApi.connectByIp(ip);
        sendJson(res, 200, result);
        return;
      }

      if (req.method === 'POST' && req.url === '/authenticate') {
        const { uid } = await readJsonBody(req);
        const result = await backendApi.authenticate(uid);
        sendJson(res, 200, result);
        return;
      }

      if (req.method === 'POST' && req.url === '/reauth') {
        const { uid } = await readJsonBody(req);
        const result = await backendApi.reauth(uid);
        sendJson(res, 200, result);
        return;
      }

      if (req.method === 'POST' && req.url === '/startCamera') {
        const { uid, encoding } = await readJsonBody(req);
        const result = await backendApi.startCamera(uid, { frameEncoding: encoding });
        sendJson(res, 200, result);
        return;
      }

      if (req.method === 'POST' && req.url === '/stopCamera') {
        const { uid } = await readJsonBody(req);
        const result = await backendApi.stopCamera(uid);
        sendJson(res, 200, result);
        return;
      }

      if (req.method === 'GET' && req.url.startsWith('/state/')) {
        const uid = decodeURIComponent(req.url.replace('/state/', ''));
        sendJson(res, 200, { uid, state: backendApi.getState(uid) });
        return;
      }

      if (req.method === 'GET' && req.url.startsWith('/camera/latest/')) {
        const uid = decodeURIComponent(req.url.replace('/camera/latest/', ''));
        sendJson(res, 200, latestFrameByUid.get(uid) || { uid, frame: null });
        return;
      }

      sendJson(res, 404, { error: 'Not found' });
    } catch (err) {
      sendJson(res, 500, {
        error: err.message,
        stack: process.env.NODE_ENV === 'development' ? err.stack : undefined
      });
    }
  });

  const port = Number(process.env.PORT || 5050);
  server.listen(port, '127.0.0.1', () => {
    console.log(`PrinterBackendApi HTTP bridge running on http://127.0.0.1:${port}`);
  });
}

main().catch(err => {
  console.error(err);
  process.exit(1);
});
