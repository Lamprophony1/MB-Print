# Guía **paso a paso** para probar `printer-backend-api` (sin romper tu instalación)

Esta guía está escrita para alguien que **no conoce Postman ni Node**.

---

## 0) Respuesta corta a tu duda principal

> “¿Tengo que hacer pull y reemplazar mi instalación local del programa para probar?”

Depende de **qué quieras probar**:

1. **Probar solo la API HTTP de ejemplo / entender el flujo** (sin impresora real):
   - **NO** necesitas tocar la instalación de MakerBot Print.
   - Podés hacer una prueba aislada con un backend “mock”.

2. **Probar contra impresora real usando la lógica real**:
   - Si tu instalación local del programa **no está vinculada** a este repo, entonces los cambios del repo **no se ejecutan** en la app instalada automáticamente.
   - En ese caso tenés 2 opciones:
     - A) hacer una integración local controlada (copiar archivos puntuales con backup), o
     - B) montar un entorno de desarrollo que cargue estos módulos desde el repo.

En resumen: **para pruebas aisladas, no reemplaces instalación**. Para pruebas reales end-to-end, en algún punto sí necesitás que la app ejecute el código nuevo.

---

## 1) Qué se agregó en este repo

- `resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/printer-backend-api.js`
- `docs/examples/printer-backend-api-http-bridge.js`

El bridge HTTP expone endpoints para usar Postman:
- `POST /connectByIp`
- `POST /authenticate`
- `POST /reauth`
- `POST /startCamera`
- `POST /stopCamera`
- `GET /state/<uid>`
- `GET /camera/latest/<uid>`
- `GET /health`

---

## 2) Opción A (recomendada para empezar): prueba aislada sin tocar instalación

Esta opción sirve para:
- aprender Postman,
- verificar que el bridge funciona,
- validar formato de requests/responses.

### 2.1 Requisitos mínimos

1. Tener **Node.js** instalado.
2. Tener este repo en tu máquina (la carpeta actual).
3. Opcional: Postman (si no, podés usar `curl`).

### 2.2 Crear backend “mock” (archivo listo para copiar/pegar)

Creá este archivo:

`local/create-printer-backend-api.mock.js`

```js
'use strict';

const EventEmitter = require('eventemitter3');

class MockBackend extends EventEmitter {
  constructor() {
    super();
    this.stateByUid = new Map();
  }

  connectByIp(ip) {
    const uid = `mock-${ip.replaceAll('.', '-')}`;
    this.stateByUid.set(uid, 'Unauthenticated');
    this.emit('state-changed', { uid, previous: undefined, current: 'Unauthenticated' });
    return { uid, name: 'Mock Printer', ip, state: 'Unauthenticated' };
  }

  authenticate(uid) {
    const previous = this.stateByUid.get(uid);
    this.stateByUid.set(uid, 'Idle');
    this.emit('state-changed', { uid, previous, current: 'Idle' });
    return { uid, name: 'Mock Printer', ip: '127.0.0.1', state: 'Idle' };
  }

  reauth(uid) {
    const previous = this.stateByUid.get(uid);
    this.stateByUid.set(uid, 'Authenticating');
    this.emit('state-changed', { uid, previous, current: 'Authenticating' });
    this.stateByUid.set(uid, 'Idle');
    this.emit('state-changed', { uid, previous: 'Authenticating', current: 'Idle' });
    return { uid, name: 'Mock Printer', ip: '127.0.0.1', state: 'Idle' };
  }

  startCamera(uid, options = {}) {
    const encoding = options.frameEncoding || 'base64';
    const onePixelJpegBase64 = '/9j/4AAQSkZJRgABAQAAAQABAAD/2wCEAAkGBxAQEBUQEBIVFhUVFRUVFRUVFRUVFRUWFhUVFRUYHSggGBolGxUVITEhJSkrLi4uFx8zODMsNygtLisBCgoKDg0OGhAQGi0lHyUtLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLf/AABEIAAEAAQMBIgACEQEDEQH/xAAXAAEBAQEAAAAAAAAAAAAAAAABAgAD/8QAFhEBAQEAAAAAAAAAAAAAAAAAAAER/9oADAMBAAIQAxAAAAGhA//EABYQAQEBAAAAAAAAAAAAAAAAAAABEf/aAAgBAQABPwCj/8QAFhEBAQEAAAAAAAAAAAAAAAAAABEB/9oACAEDAQE/AV//xAAXEQEAAwAAAAAAAAAAAAAAAAABABEh/9oACAECAQE/ATf/xAAWEAEBAQAAAAAAAAAAAAAAAAABABH/2gAIAQEABj8Cr//EABcQAQEBAQAAAAAAAAAAAAAAAAERACH/2gAIAQEAAT8h0S5//9oADAMBAAIAAwAAABAf/8QAFhEBAQEAAAAAAAAAAAAAAAAAABEB/9oACAEDAQE/EJf/xAAXEQADAQAAAAAAAAAAAAAAAAAAAREh/9oACAECAQE/EI0f/8QAFhABAQEAAAAAAAAAAAAAAAAAARAR/9oACAEBAAE/EHCRP//Z';
    const frame = encoding === 'binary' ? Buffer.from(onePixelJpegBase64, 'base64') : `data:image/jpeg;base64,${onePixelJpegBase64}`;

    this.emit('camera-frame', { uid, encoding, frame });
    return { uid, active: true, encoding };
  }

  stopCamera(uid) {
    return { uid, active: false };
  }

  getState(uid) {
    return this.stateByUid.get(uid) || 'Offline';
  }
}

module.exports = () => new MockBackend();
```

### 2.3 Levantar bridge HTTP

En terminal, parado en la raíz del repo:

```bash
BACKEND_FACTORY=./local/create-printer-backend-api.mock.js PORT=5050 node docs/examples/printer-backend-api-http-bridge.js
```

Si está bien, vas a ver:

`PrinterBackendApi HTTP bridge running on http://127.0.0.1:5050`

### 2.4 Probar sin Postman (solo con curl)

```bash
curl http://127.0.0.1:5050/health
```

```bash
curl -X POST http://127.0.0.1:5050/connectByIp -H "Content-Type: application/json" -d '{"ip":"192.168.1.30"}'
```

Guardá el `uid` de la respuesta y seguí:

```bash
curl -X POST http://127.0.0.1:5050/authenticate -H "Content-Type: application/json" -d '{"uid":"mock-192-168-1-30"}'
```

```bash
curl -X POST http://127.0.0.1:5050/startCamera -H "Content-Type: application/json" -d '{"uid":"mock-192-168-1-30","encoding":"base64"}'
```

```bash
curl http://127.0.0.1:5050/camera/latest/mock-192-168-1-30
```

```bash
curl -X POST http://127.0.0.1:5050/stopCamera -H "Content-Type: application/json" -d '{"uid":"mock-192-168-1-30"}'
```

### 2.5 Probar con Postman (click a click)

1. Abrí Postman.
2. Botón **New** → **HTTP Request**.
3. Método: `GET`.
4. URL: `http://127.0.0.1:5050/health`.
5. Click **Send** (debe devolver `{ "ok": true }`).
6. Duplicá la pestaña y cambiá:
   - Método: `POST`
   - URL: `http://127.0.0.1:5050/connectByIp`
   - Tab **Body** → **raw** → tipo **JSON**
   - Pegá:
     ```json
     { "ip": "192.168.1.30" }
     ```
   - **Send**.
7. Copiá el `uid` de la respuesta.
8. Repetí para `/authenticate`, `/startCamera`, etc., cambiando body según endpoint.

---

## 3) Opción B: prueba real con impresora (sin “romper” instalación)

Si querés prueba real, el backend tiene que usar tu `printerManager` real.

### 3.1 Estrategia segura recomendada

1. **No toques primero Program Files directo**.
2. Hacé una copia de tu instalación (o snapshot).
3. Trabajá sobre la copia para validar.
4. Recién cuando funcione, replicás en instalación principal.

### 3.2 ¿Necesito “pull + reemplazar instalación”?

- **Pull en tu repo local**: sí, para tener los cambios.
- **Reemplazar instalación**: solo si querés que la app instalada ejecute esos cambios.
- Para pruebas aisladas (mock/bridge), **no** hace falta reemplazar nada.

### 3.3 Flujo práctico sugerido

1. En tu máquina de trabajo:
   - `git pull` en este repo.
2. Hacer prueba aislada (sección 2).
3. Si eso pasa, preparar factory real (con tu inicialización de `printerManager`).
4. Levantar bridge con factory real y validar endpoints contra impresora.
5. Solo si necesitás validar dentro de la app instalada, aplicar copia controlada de archivos (con backup previo).

---

## 4) ¿Cuál alternativa es mejor que Postman?

Para producto, mejor:

- Exponer en Electron main con `ipcMain.handle(...)`.
- Consumir desde renderer con `ipcRenderer.invoke(...)`.
- Agregar tests Node de integración para estados/eventos.

Postman te sirve para diagnóstico manual rápido, pero IPC/tests reflejan mejor el comportamiento real de la app.

---

## 5) Errores comunes

- `Missing BACKEND_FACTORY env var`: faltó variable de entorno.
- `Invalid JSON body`: body mal formado.
- `Unknown printer uid`: usaste un UID que no existe en esa sesión.
- No llegan frames: no autenticado, cámara no activa, o endpoint de consulta equivocado.
