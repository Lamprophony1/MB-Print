# Guía de prueba del backend `printer-backend-api`

Esta guía te da 2 caminos:

1. **Postman** (rápido para QA manual) usando un *HTTP bridge* de desarrollo.
2. **Alternativa recomendada** para integración real: probarlo por **IPC de Electron** o tests Node, ya que `PrinterBackendApi` es una API interna de proceso main.

---

## 1) Opción Postman (con HTTP bridge)

### 1.1. ¿Por qué hace falta un bridge?
`printer-backend-api.js` no expone HTTP por sí solo; expone métodos JS internos.
Postman necesita endpoints HTTP, por eso se agrega el puente:

- `docs/examples/printer-backend-api-http-bridge.js`

Este script publica endpoints HTTP y llama internamente a:
- `connectByIp(ip)`
- `authenticate(uid)`
- `reauth(uid)`
- `startCamera(uid)`
- `stopCamera(uid)`

### 1.2. Crear tu factory local
Creá un módulo local (ejemplo `./local/create-printer-backend-api.js`) que devuelva una instancia lista de `PrinterBackendApi` con tu `printerManager` real inicializado.

Contrato esperado del factory:
- Exportar **función** (sync/async) que retorne la instancia, **o**
- Exportar directamente la instancia.

### 1.3. Levantar el bridge
```bash
BACKEND_FACTORY=./local/create-printer-backend-api.js PORT=5050 node docs/examples/printer-backend-api-http-bridge.js
```

Health check:
```bash
curl http://127.0.0.1:5050/health
```

### 1.4. Requests en Postman
Base URL: `http://127.0.0.1:5050`

1) **Connect by IP**
- `POST /connectByIp`
- Body JSON:
```json
{ "ip": "192.168.1.30" }
```

2) **Authenticate**
- `POST /authenticate`
- Body JSON:
```json
{ "uid": "<uid_obtenido_en_connect>" }
```

3) **Reauth**
- `POST /reauth`
- Body JSON:
```json
{ "uid": "<uid>" }
```

4) **Start camera**
- `POST /startCamera`
- Body JSON (base64):
```json
{ "uid": "<uid>", "encoding": "base64" }
```

- Body JSON (binario serializado como base64 en respuesta de bridge):
```json
{ "uid": "<uid>", "encoding": "binary" }
```

5) **Leer último frame capturado**
- `GET /camera/latest/<uid>`

6) **State actual**
- `GET /state/<uid>`

7) **Stop camera**
- `POST /stopCamera`
- Body JSON:
```json
{ "uid": "<uid>" }
```

---

## 2) Alternativa mejor para producto: IPC Electron o tests Node

Para producción/integración, la estrategia más sólida es:

- Exponer estos métodos por `ipcMain.handle(...)`.
- Consumirlos desde renderer con `ipcRenderer.invoke(...)`.
- En tests Node, mockear `printerManager` y validar transiciones de estado/eventos.

Ventajas sobre Postman:
- Menos capas (sin HTTP bridge intermedio).
- Reproduce mejor el flujo real de la app Electron.
- Menos riesgo de falsos positivos por diferencias de serialización HTTP.

---

## Troubleshooting rápido

- **`Missing BACKEND_FACTORY env var`**: no seteaste el path al factory.
- **`Unknown printer uid`**: el UID no está en caché del `printerManager`.
- **`Cannot reauthenticate offline printer`**: no hay `printerInfo` activo para ese UID.
- **No llegan frames**: validar autenticación previa, estado del equipo y `startCamera` exitoso.
