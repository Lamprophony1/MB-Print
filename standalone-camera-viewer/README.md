# Standalone Camera Viewer (real printer, sin modificar MB internamente)

Esta app es **independiente**. No modifica `MB-support-plugin` ni requiere copiar DLL/EXE al instalador.

## Qué hace

- Descubre impresora por IP (`findByIp`).
- Autentica (`connectPrinter` / opcional `reconnectPrinter`).
- Inicia stream de cámara (`setCameraFrameNotification` + `RequestCameraStream`).
- Muestra frames en navegador en tiempo real (SSE).

## Requisitos

- Node.js 18+
- Acceso de red a la impresora
- Módulo Croissant accesible desde disco (`croissant.js` de MB-support-plugin)

## Variables de entorno

- `PORT` (default `6060`)
- `HOST` (default `127.0.0.1`)
- `CROISSANT_MODULE_PATH` (ruta al `croissant.js` real)

Si no se define `CROISSANT_MODULE_PATH`, usa por defecto el del repo:

`resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/croissant.js`

## Ejecutar

```bash
node standalone-camera-viewer/server.js
```

Con ruta explícita al croissant de tu instalación Windows:

```powershell
$env:CROISSANT_MODULE_PATH='C:\Program Files\MakerBot\MakerBotPrint\resources\app.asar.unpacked\node_modules\MB-support-plugin\lib\croissant.js'
$env:PORT='6060'
node .\standalone-camera-viewer\server.js
```

Abrir en navegador:

- `http://127.0.0.1:6060`

## API HTTP

- `GET /api/health`
- `POST /api/connectByIp` body `{ "ip": "192.168.1.30" }`
- `POST /api/authenticate` body `{ "uid": "...", "mode": "connect|reauth" }`
- `POST /api/startCamera` body `{ "uid": "...", "encoding": "base64|binary" }`
- `POST /api/stopCamera` body `{ "uid": "..." }`
- `GET /api/state/<uid>`
- `GET /api/camera/latest/<uid>`
- `GET /api/camera/stream/<uid>` (SSE)

## Nota importante

- Esta app está diseñada para experimentar con ingeniería inversa de forma aislada.
- Si tu instalación de MakerBot quedó dañada (0xc000012f), primero reinstalá limpio y no vuelvas a copiar binarios (`node.dll`, `ffmpeg.dll`, `.exe`) desde este repo.


## Troubleshooting: "ERROR ... Failed to fetch"

Si en la UI ves `ERROR connect: Failed to fetch` o similares:

1. Ejecutá primero health desde terminal:
   ```bash
   curl http://127.0.0.1:6060/api/health
   ```
2. En la UI, configurá **API Base URL** correcta (ejemplo `http://127.0.0.1:6060`).
3. No abras `index.html` como archivo suelto (`file://...`) sin API Base correcta.
4. Si corrés server en otra máquina/VM, iniciá con `HOST=0.0.0.0` y usá IP real de esa máquina en API Base URL.
5. Verificá firewall/antivirus bloqueando el puerto.

Ejemplo (Linux/macOS):
```bash
HOST=0.0.0.0 PORT=6060 node standalone-camera-viewer/server.js
```

Ejemplo (PowerShell):
```powershell
$env:HOST='0.0.0.0'
$env:PORT='6060'
node .\standalone-camera-viewer\server.js
```
