'use strict';

const http = require('http');
const path = require('path');
const fs = require('fs');

const PORT = Number(process.env.PORT || 6060);
const HOST = process.env.HOST || '127.0.0.1';
const CROISSANT_MODULE_PATH = process.env.CROISSANT_MODULE_PATH || path.resolve(__dirname, '..', 'resources', 'app.asar.unpacked', 'node_modules', 'MB-support-plugin', 'lib', 'croissant.js');

const stateEnum = {
  Offline: 'Offline',
  Unauthenticated: 'Unauthenticated',
  Authenticating: 'Authenticating',
  Idle: 'Idle'
};

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
    'Content-Type': 'application/json; charset=utf-8',
    'Content-Length': Buffer.byteLength(body),
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Methods': 'GET,POST,OPTIONS',
    'Access-Control-Allow-Headers': 'Content-Type'
  });
  res.end(body);
}

function normalizeConnectResult(connectResult) {
  if (Array.isArray(connectResult)) {
    return {
      printer: connectResult[0] || null,
      authInfo: connectResult[1] || null
    };
  }

  return {
    printer: connectResult || null,
    authInfo: null
  };
}

function toBuffer(frame) {
  if (Buffer.isBuffer(frame)) return frame;
  if (frame instanceof Uint8Array) return Buffer.from(frame);
  if (ArrayBuffer.isView(frame)) return Buffer.from(frame.buffer);
  if (frame instanceof ArrayBuffer) return Buffer.from(new Uint8Array(frame));
  if (typeof frame === 'string') {
    const b64 = frame.includes(',') ? frame.split(',').pop() : frame;
    return Buffer.from(b64, 'base64');
  }

  return Buffer.alloc(0);
}

function loadCroissant() {
  const croissant = require(CROISSANT_MODULE_PATH);
  if (!croissant || typeof croissant.PrinterFinder !== 'function') {
    throw new Error(`Croissant module at ${CROISSANT_MODULE_PATH} does not export PrinterFinder`);
  }

  return croissant;
}

async function main() {
  let finder = null;

  function getFinder() {
    if (!finder) {
      const croissant = loadCroissant();
      finder = new croissant.PrinterFinder();
    }

    return finder;
  }

  const printers = new Map();
  const sseClientsByUid = new Map();

  function getEntryOrThrow(uid) {
    const entry = printers.get(uid);
    if (!entry) throw new Error(`Unknown printer uid ${uid}`);
    return entry;
  }

  function setState(uid, state) {
    const entry = getEntryOrThrow(uid);
    entry.state = state;
  }

  function broadcastFrame(uid, payload) {
    const clients = sseClientsByUid.get(uid);
    if (!clients) return;

    const data = `event: frame\ndata: ${JSON.stringify(payload)}\n\n`;

    clients.forEach(res => {
      res.write(data);
    });
  }

  async function connectByIp(ip) {
    if (!ip) throw new Error('ip is required');

    const printerInfo = await getFinder().findByIp(ip);
    const uid = printerInfo.uid;

    printers.set(uid, {
      uid,
      name: printerInfo.name,
      ip,
      printerInfo,
      printer: null,
      authInfo: null,
      state: stateEnum.Unauthenticated,
      lastFrameBase64: null,
      cameraEncoding: 'base64'
    });

    return printers.get(uid);
  }

  async function authenticate(uid, mode = 'connect') {
    const entry = getEntryOrThrow(uid);

    setState(uid, stateEnum.Authenticating);

    let connectResult;

    if (mode === 'reauth') {
      connectResult = entry.authInfo ? await getFinder().reconnectPrinter(entry.printerInfo, entry.authInfo) : await getFinder().reconnectPrinter(entry.printerInfo);
    } else {
      connectResult = await getFinder().connectPrinter(entry.printerInfo);
    }

    const normalized = normalizeConnectResult(connectResult);
    entry.printer = normalized.printer;
    if (normalized.authInfo) entry.authInfo = normalized.authInfo;

    setState(uid, stateEnum.Idle);

    return entry;
  }

  async function startCamera(uid, encoding = 'base64') {
    const entry = getEntryOrThrow(uid);

    if (!entry.printer) {
      throw new Error('Printer is not authenticated yet. Call /api/authenticate first.');
    }

    entry.cameraEncoding = encoding === 'binary' ? 'binary' : 'base64';

    entry.printer.setCameraFrameNotification(frame => {
      const buf = toBuffer(frame);
      const b64 = buf.toString('base64');
      entry.lastFrameBase64 = b64;

      const payload = {
        uid,
        encoding: entry.cameraEncoding,
        frame: entry.cameraEncoding === 'binary' ? b64 : `data:image/jpeg;base64,${b64}`,
        ts: new Date().toISOString()
      };

      broadcastFrame(uid, payload);
    });

    await entry.printer.RequestCameraStream();

    return {
      uid,
      active: true,
      encoding: entry.cameraEncoding
    };
  }

  async function stopCamera(uid) {
    const entry = getEntryOrThrow(uid);
    if (!entry.printer) throw new Error('Printer is not authenticated');

    await entry.printer.EndCameraStream();
    if (typeof entry.printer.unsetCameraFrameNotification === 'function') {
      entry.printer.unsetCameraFrameNotification();
    }

    return { uid, active: false };
  }

  function routeStatic(req, res) {
    if (req.method !== 'GET') return false;

    const pathname = req.url === '/' ? '/index.html' : req.url;
    const filePath = path.join(__dirname, 'public', pathname);

    if (!filePath.startsWith(path.join(__dirname, 'public'))) {
      sendJson(res, 403, { error: 'Forbidden' });
      return true;
    }

    if (!fs.existsSync(filePath)) {
      return false;
    }

    const ext = path.extname(filePath);
    const contentType = ext === '.html' ? 'text/html; charset=utf-8' : 'text/plain; charset=utf-8';
    const file = fs.readFileSync(filePath);

    res.writeHead(200, {
      'Content-Type': contentType,
      'Content-Length': file.length,
      'Access-Control-Allow-Origin': '*'
    });

    res.end(file);
    return true;
  }

  const server = http.createServer(async (req, res) => {
    try {

      if (req.method === 'OPTIONS') {
        res.writeHead(204, {
          'Access-Control-Allow-Origin': '*',
          'Access-Control-Allow-Methods': 'GET,POST,OPTIONS',
          'Access-Control-Allow-Headers': 'Content-Type'
        });
        res.end();
        return;
      }

      if (routeStatic(req, res)) return;

      if (req.method === 'GET' && req.url === '/api/health') {
        sendJson(res, 200, {
          ok: true,
          croissantModulePath: CROISSANT_MODULE_PATH
        });
        return;
      }

      if (req.method === 'POST' && req.url === '/api/connectByIp') {
        const { ip } = await readJsonBody(req);
        const entry = await connectByIp(ip);
        sendJson(res, 200, {
          uid: entry.uid,
          name: entry.name,
          ip: entry.ip,
          state: entry.state
        });
        return;
      }

      if (req.method === 'POST' && req.url === '/api/authenticate') {
        const { uid, mode } = await readJsonBody(req);
        const entry = await authenticate(uid, mode || 'connect');
        sendJson(res, 200, {
          uid: entry.uid,
          state: entry.state,
          hasAuthInfo: !!entry.authInfo
        });
        return;
      }

      if (req.method === 'POST' && req.url === '/api/startCamera') {
        const { uid, encoding } = await readJsonBody(req);
        const result = await startCamera(uid, encoding || 'base64');
        sendJson(res, 200, result);
        return;
      }

      if (req.method === 'POST' && req.url === '/api/stopCamera') {
        const { uid } = await readJsonBody(req);
        const result = await stopCamera(uid);
        sendJson(res, 200, result);
        return;
      }

      if (req.method === 'GET' && req.url.startsWith('/api/state/')) {
        const uid = decodeURIComponent(req.url.replace('/api/state/', ''));
        const entry = getEntryOrThrow(uid);
        sendJson(res, 200, {
          uid,
          state: entry.state
        });
        return;
      }

      if (req.method === 'GET' && req.url.startsWith('/api/camera/latest/')) {
        const uid = decodeURIComponent(req.url.replace('/api/camera/latest/', ''));
        const entry = getEntryOrThrow(uid);
        sendJson(res, 200, {
          uid,
          frame: entry.lastFrameBase64 ? `data:image/jpeg;base64,${entry.lastFrameBase64}` : null
        });
        return;
      }

      if (req.method === 'GET' && req.url.startsWith('/api/camera/stream/')) {
        const uid = decodeURIComponent(req.url.replace('/api/camera/stream/', ''));
        getEntryOrThrow(uid);

        res.writeHead(200, {
          'Content-Type': 'text/event-stream',
          'Cache-Control': 'no-cache',
          Connection: 'keep-alive',
          'Access-Control-Allow-Origin': '*'
        });

        res.write('\n');

        if (!sseClientsByUid.has(uid)) {
          sseClientsByUid.set(uid, new Set());
        }

        const set = sseClientsByUid.get(uid);
        set.add(res);

        req.on('close', () => {
          set.delete(res);
          if (!set.size) sseClientsByUid.delete(uid);
        });

        return;
      }

      sendJson(res, 404, { error: 'Not found' });
    } catch (err) {
      sendJson(res, 500, {
        error: err.message
      });
    }
  });

  server.listen(PORT, HOST, () => {
    console.log(`Standalone camera viewer running on http://${HOST}:${PORT}`);
  });
}

main().catch(err => {
  console.error(err);
  process.exit(1);
});
