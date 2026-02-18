'use strict';

const http = require('http');
const path = require('path');
const fs = require('fs');

const PORT = Number(process.env.PORT || 6060);
const HOST = process.env.HOST || '127.0.0.1';
const CROISSANT_MODULE_PATH = process.env.CROISSANT_MODULE_PATH || path.resolve(__dirname, '..', 'resources', 'app.asar.unpacked', 'node_modules', 'MB-support-plugin', 'lib', 'croissant.js');
const FINDER_USERNAME = process.env.FINDER_USERNAME || 'ANON';
// Same public client secret used by MB-support-plugin Thingiverse auth flow
const DEFAULT_FINDER_CLIENT_SECRET = 'c30f532bcc67bb65d3476daedc0e60f4';
const FINDER_CLIENT_SECRET = process.env.FINDER_CLIENT_SECRET || DEFAULT_FINDER_CLIENT_SECRET;

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

function looksLikePrinter(candidate) {
  if (!candidate || typeof candidate !== 'object') return false;

  return typeof candidate.setCameraFrameNotification === 'function'
    || typeof candidate.RequestCameraStream === 'function'
    || typeof candidate.EndCameraStream === 'function';
}

function normalizeConnectResult(connectResult) {
  if (Array.isArray(connectResult)) {
    const first = connectResult[0] || null;
    const second = connectResult[1] || null;

    if (looksLikePrinter(first)) {
      return { printer: first, authInfo: second };
    }

    if (looksLikePrinter(second)) {
      return { printer: second, authInfo: first };
    }

    return {
      printer: first,
      authInfo: second
    };
  }

  if (connectResult && typeof connectResult === 'object') {
    if (looksLikePrinter(connectResult.printer)) {
      return {
        printer: connectResult.printer,
        authInfo: connectResult.authInfo || connectResult.auth_info || null
      };
    }

    if (looksLikePrinter(connectResult.connection)) {
      return {
        printer: connectResult.connection,
        authInfo: connectResult.authInfo || connectResult.auth_info || null
      };
    }
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
  try {
    const croissant = require(CROISSANT_MODULE_PATH);
    if (!croissant || typeof croissant.PrinterFinder !== 'function') {
      throw new Error(`Croissant module at ${CROISSANT_MODULE_PATH} does not export PrinterFinder`);
    }

    return croissant;
  } catch (err) {
    const msg = String(err && err.message || err || 'Unknown error');

    if (msg.includes('NODE_MODULE_VERSION')) {
      throw new Error(
        `Croissant native module ABI mismatch. Tu runtime Node no coincide con el binario croissantjs.node.
`
        + `Comandos válidos:
`
        + `  Opción A (desde repo root): npm --prefix standalone-camera-viewer run start:legacy-win
`
        + `  Opción B (entrando a la carpeta):
`
        + `    1) cd standalone-camera-viewer
`
        + `    2) npm install
`
        + `    3) npm run start:legacy-win (Windows)
`
        + `       o npm run start:legacy-unix (Linux/macOS)
`
        + `No ejecutes luego "node standalone-camera-viewer/server.js" desde dentro de standalone-camera-viewer,
`
        + `porque eso duplica la ruta y falla con MODULE_NOT_FOUND.
`
        + `Si sigue fallando por Expected X / got Y, en Windows usá: npm run start:makerbot-win
`
        + `Detalle original: ${msg}`
      );
    }

    throw err;
  }
}


function main() {
  let finder = null;

  function getFinder() {
    if (!finder) {
      const croissant = loadCroissant();
      finder = new croissant.PrinterFinder();

      if (FINDER_CLIENT_SECRET && typeof finder.setClientSecret === 'function') {
        finder.setClientSecret(FINDER_CLIENT_SECRET);
      }

      if (typeof finder.setUsernameOnly === 'function') {
        finder.setUsernameOnly(FINDER_USERNAME);
      }
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

  function connectByIp(ip) {
    if (!ip) return Promise.reject(new Error('ip is required'));

    return Promise.resolve(getFinder().findByIp(ip)).then(printerInfo => {
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
    });
  }

  function authenticate(uid, mode) {
    const resolvedMode = mode || 'connect';
    const entry = getEntryOrThrow(uid);

    setState(uid, stateEnum.Authenticating);

    const connectPromise = resolvedMode === 'reauth'
      ? (entry.authInfo ? getFinder().reconnectPrinter(entry.printerInfo, entry.authInfo) : getFinder().reconnectPrinter(entry.printerInfo))
      : getFinder().connectPrinter(entry.printerInfo);

    return Promise.resolve(connectPromise).then(connectResult => {
      const normalized = normalizeConnectResult(connectResult);
      entry.printer = normalized.printer;
      if (normalized.authInfo) entry.authInfo = normalized.authInfo;

      setState(uid, stateEnum.Idle);
      return entry;
    });
  }

  function startCamera(uid, encoding) {
    const resolvedEncoding = encoding || 'base64';
    const entry = getEntryOrThrow(uid);

    if (!entry.printer) {
      return Promise.reject(new Error('Printer is not authenticated yet. Call /api/authenticate first.'));
    }

    if (typeof entry.printer.setCameraFrameNotification !== 'function') {
      const availableKeys = Object.keys(entry.printer || {}).slice(0, 30).join(', ');
      return Promise.reject(new Error(`Connected object is not a camera-capable printer. Available keys: ${availableKeys}`));
    }

    entry.cameraEncoding = resolvedEncoding === 'binary' ? 'binary' : 'base64';

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

    return Promise.resolve(entry.printer.RequestCameraStream()).then(() => ({
      uid,
      active: true,
      encoding: entry.cameraEncoding
    }));
  }

  function stopCamera(uid) {
    const entry = getEntryOrThrow(uid);
    if (!entry.printer) return Promise.reject(new Error('Printer is not authenticated'));

    return Promise.resolve(entry.printer.EndCameraStream()).then(() => {
      if (typeof entry.printer.unsetCameraFrameNotification === 'function') {
        entry.printer.unsetCameraFrameNotification();
      }

      return { uid, active: false };
    });
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

  const server = http.createServer((req, res) => {
    const fail = err => {
      sendJson(res, 500, {
        error: err && err.message ? err.message : String(err)
      });
    };

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
          croissantModulePath: CROISSANT_MODULE_PATH,
          runtime: {
            node: process.versions.node,
            modules: process.versions.modules,
            electron: process.versions.electron || null
          },
          authContext: {
            username: FINDER_USERNAME,
            hasClientSecret: !!FINDER_CLIENT_SECRET
          }
        });
        return;
      }

      if (req.method === 'POST' && req.url === '/api/connectByIp') {
        readJsonBody(req).then(body => connectByIp(body.ip)).then(entry => {
          sendJson(res, 200, {
            uid: entry.uid,
            name: entry.name,
            ip: entry.ip,
            state: entry.state
          });
        }).catch(fail);
        return;
      }

      if (req.method === 'POST' && req.url === '/api/authenticate') {
        readJsonBody(req).then(body => authenticate(body.uid, body.mode || 'connect')).then(entry => {
          sendJson(res, 200, {
            uid: entry.uid,
            state: entry.state,
            hasAuthInfo: !!entry.authInfo
          });
        }).catch(err => {
          const msg = String(err && err.message || err || '');
          if (msg.includes('Neither thingiverse token nor client secret set')) {
            fail(new Error('Neither thingiverse token nor client secret set. Verificá FINDER_CLIENT_SECRET (o dejalo por default en este server) y reiniciá.'));
            return;
          }
          fail(err);
        });
        return;
      }

      if (req.method === 'POST' && req.url === '/api/startCamera') {
        readJsonBody(req).then(body => startCamera(body.uid, body.encoding || 'base64')).then(result => {
          sendJson(res, 200, result);
        }).catch(fail);
        return;
      }

      if (req.method === 'POST' && req.url === '/api/stopCamera') {
        readJsonBody(req).then(body => stopCamera(body.uid)).then(result => {
          sendJson(res, 200, result);
        }).catch(fail);
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
      fail(err);
    }
  });

  server.listen(PORT, HOST, () => {
    console.log(`Standalone camera viewer running on http://${HOST}:${PORT}`);
  });
}

try {
  main();
} catch (err) {
  console.error(err);
  process.exit(1);
}
