# Reconstrucción de flujo UI/Auth (MB Print)

## 1) Recuperación de `resources/app.asar` real

- `resources/app.asar` en este checkout es un puntero Git LFS (no binario ASAR):
  - `oid sha256:44215a8ba04bcf53ac7fc84ea9578345633017b22501ef4b0dbdf4497969c237`
  - `size 130216849`
- El repositorio local no tiene remoto configurado (`git remote -v` vacío), por lo que no fue posible hacer `git lfs fetch/pull` contra el origen para materializar ese objeto.
- Además, el entorno devolvió `403` al intentar acceso a GitHub, así que tampoco se pudo inferir/usar el remoto original desde red.

**Estado:** no recuperable en este entorno, pero sí se pudo trabajar con el contenido ya descomprimido existente en `resources/app.asar.unpacked/`.

---

## 2) Módulos UI/Auth localizados

### Entrada de IP / alta de impresora

- Método de alta por IP en provider:
  - `addPrinter(ip, callback)` limpia prefijo `ip://` y delega a `printerManager.addByIp(ip)`.
  - Archivo: `resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/printer-service-provider.js`

- Resolución por IP:
  - `addByIp(ip)` llama a `finder.findByIp(ip)`, construye/reusa printer y luego intenta auth desde credenciales almacenadas.
  - Archivo: `resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/printerManager.js`

### Auth (incluye botón “Autorizar” en impresora)

- Entrada al flujo auth desde UI panel:
  - `PrinterDetailsPanel` dispara `actions.printer.authenticate(this.props.printer)` al montar y al cambiar de impresora no autenticada.
  - Archivo: `resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/ui/views/PrinterDetailsPanel.js`

- Orquestación auth:
  - `printerManager.authenticate(printer)` decide:
    1. `requestNewAuth` (si impresora de red “nueva” para usuario actual), o
    2. `authFromStored` (reauth), con fallback a `requestNewAuth` si `Unauthorized`.
  - Archivo: `resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/printerManager.js`

- UI de autorización:
  - En estado `PrinterStateEnum.Authenticating`, `cameraFeed` muestra imagen de instrucción `authorize_*.png` (Rep5/Mini/6th gen), que corresponde al UX de “presiona Autorizar en la impresora”.
  - Archivo: `resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/ui/views/cameraFeed.js`

### Cámara

- Solicitud de stream:
  - `makerbot-printer.getCameraFeed` registra callback y ejecuta `invoke('RequestCameraStream')`.
  - Stop con `invoke('EndCameraStream')`.
  - Archivo: `resources/app.asar.unpacked/node_modules/MB-support-plugin/lib/makerbot-printer.js`

---

## 3) Flujo exacto: “Ingresar IP” ➜ botón “Autorizar”

1. **Usuario ingresa IP** en UI (ruta exacta de vista no está en el unpacked disponible, pero el backend espera `addPrinter(ip)`).
2. `addPrinter` normaliza `ip://` y llama `addByIp`.
3. `addByIp` hace `finder.findByIp(ip)` y obtiene `printerInfo`.
4. Si no hay printer en caché apropiada, crea una nueva (`_createPrinter` + `ConnectionManager`).
5. Si el printer no está autenticado, intenta `authFromStored(printerInfo)`:
   - Para NETWORK sin auth previa del usuario actual puede lanzar `No stored auth info for network printer`.
6. En el panel de detalles (`PrinterDetailsPanel`), la UI dispara `actions.printer.authenticate(printer)`.
7. `printerManager.authenticate`:
   - Si network bot “nuevo” (`uid` no está en `_currentPrinters`): `requestNewAuth`.
   - Si no, intenta `authFromStored`; si responde `Unauthorized`, hace fallback a `requestNewAuth`.
8. `requestNewAuth` para NETWORK pone estado `Authenticating` y hace `finder.connectPrinter(printerInfo)`.
9. Mientras estado = `Authenticating`, `cameraFeed` renderiza la imagen de “authorize” (UX visual para que el usuario pulse **Autorizar** en la impresora).
10. Al confirmar físicamente en impresora, la conexión completa en `_doAuthenticate`, se cambia conexión activa, se actualiza store y el estado deja de `Authenticating` (normalmente a operativo/autenticado).

---

## 4) Payloads y transiciones de estado (para reproducir UX original)

## Payloads observables en código

### Alta por IP

```js
addPrinter(ip)
// ip esperado: "ip://<ipv4>" o "<ipv4>"
// normalización: ip = ip.replace('ip://', '')
```

### Identidad de conexión (`printerInfo`)

Campos usados recurrentemente:

```js
{
  uid: string,
  name: string,
  address: string,      // p.ej. "tcp:192.168.0.10:9999" o "192.168.0.10"
  connType: "NETWORK" | "USB" | "REFLECTOR" | ...,
  info: { bot_type: string, ... }
}
```

### Persistencia auth/local

`stored_printers_manager.updateStoredInfo(printerInfo, machineConfig, authInfo, userToken)` persiste:

```js
{
  printer_info: { uid, name, address, info },
  machineConfig,
  userTokens?: [token],
  authInfo?: <blob devuelto por conexión/auth>
}
```

> Para sesiones anónimas, `authInfo` se almacena explícitamente y se reutiliza en `authFromStored`.

### Cámara

```js
invoke('RequestCameraStream')
invoke('EndCameraStream')
```

## Transiciones de estado (mínimas para UX fiel)

Para el trayecto pedido:

1. `Unauthenticated` (o sin sesión válida)
2. `Authenticating` (al entrar en `requestNewAuth` para NETWORK)
3. UI muestra imagen `authorize_*.png` (instrucción de pulsar botón en hardware)
4. Si autoriza correctamente: estado autenticado/operativo (`isAuthenticated() === true`, panel habilita acciones)
5. Si falla/cancela: vuelve a `Unauthenticated` y se propaga error de auth (`GeneralAuthError`)

Además, para reintentos:

- `Reauthenticating` se usa en rutas de reconexión/auth desde credenciales previas.

---

## Script corto de reproducción UX (black-box)

1. Abrir panel de impresora por IP.
2. Ingresar `ip://<IP_IMPRESORA>`.
3. Esperar transición a estado auth:
   - verificar visual de imagen `authorize_*`.
4. Pulsar **Autorizar** en la impresora física.
5. Confirmar que desaparece pantalla de autorización y aparecen controles normales (indicador autenticado).
6. (Opcional) abrir cámara para validar `RequestCameraStream` post-auth.

---

## Limitaciones

- No fue posible recuperar el `app.asar` binario original desde remoto (falta remoto y bloqueo de red a GitHub en este entorno).
- El análisis se basó en `resources/app.asar.unpacked` ya disponible localmente, que sí contiene los módulos funcionales de auth/UI necesarios para trazar el flujo solicitado.
