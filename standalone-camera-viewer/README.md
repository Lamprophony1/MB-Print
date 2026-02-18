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

Desde **repo root**:

```bash
node standalone-camera-viewer/server.js
```

Desde la carpeta `standalone-camera-viewer`:

```bash
node server.js
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


## Error específico: `NODE_MODULE_VERSION 48` vs `137`

Si ves este error:

- `croissantjs.node was compiled against NODE_MODULE_VERSION 48`
- `This version of Node.js requires NODE_MODULE_VERSION 137`

significa que estás ejecutando con un Node moderno y el binario nativo de Croissant fue compilado para runtime viejo (ABI 48).

### Solución práctica (sin tocar MakerBot instalado)

1. Entrá a la carpeta standalone:
   ```bash
   cd standalone-camera-viewer
   ```
2. Instalá dependencias (descarga Electron legacy):
   ```bash
   npm install
   ```
3. Ejecutá con runtime legacy:

   **Windows (PowerShell/cmd):**
   ```bash
   npm run start:legacy-win
   ```

   **Linux/macOS:**
   ```bash
   npm run start:legacy-unix
   ```

   Alternativa desde repo root (sin hacer `cd`):
   ```bash
   npm --prefix standalone-camera-viewer run start:legacy-win
   ```

4. En navegador abrí `http://127.0.0.1:6060` y corré primero **Health check**.

5. Verificá en el JSON de `/api/health` el campo `runtime.modules`:
   - Si usás Node moderno vas a ver algo como `137`.
   - Para Croissant ABI viejo necesitás runtime compatible con `48`.

> Nota: esto mantiene la app independiente; no hace falta reemplazar DLL/EXE de MakerBot.


### Error de ruta común

Si ya estás en `standalone-camera-viewer`, **no** corras:

```bash
node standalone-camera-viewer/server.js
```

porque busca una ruta duplicada (`.../standalone-camera-viewer/standalone-camera-viewer/server.js`).

En ese caso el comando correcto es:

```bash
node server.js
```
