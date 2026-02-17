# Diagnóstico de autenticación Croissant

## Objetivo
Agregar trazabilidad estructurada en el flujo de conexión/autenticación de impresoras para facilitar soporte y análisis de incidentes sin exponer secretos.

## Capa de diagnóstico implementada
Se extendió `lib/croissant.js` para envolver métodos críticos del módulo nativo `croissant` con logs JSON estructurados:

- `connectPrinter`
- `reconnectPrinter`
- Métodos del objeto printer retornado:
  - `Authorize`
  - `Authenticate`
  - `Reauthorize`

### Formato de log
Todos los eventos se emiten vía `console.log` con prefijo:

- `[croissant-diagnostic]`

Estructura base:

```json
{
  "type": "croissant_call",
  "method": "connectPrinter",
  "stage": "input|output|error",
  "durationMs": 123,
  "args": [...],
  "result": {...},
  "mappedErrorState": "UnauthorizedError|AuthRejectedError|AuthTimedOutError"
}
```

### Política de sanitización
Para evitar fuga de secretos, la capa enmascara:

- llaves con nombres sensibles: `token`, `secret`, `password`, `authorization`, `authInfo`, `auth_code`, `code`
- cadenas tipo `Bearer ...`

Reemplazo utilizado: `[REDACTED]`.

## Secuencia técnica de autenticación

1. `printerManager.requestNewAuth(...)` usa `finder.connectPrinter(printerInfo)`.
2. `printerManager.authFromStored(...)` usa `finder.reconnectPrinter(printerInfo, authInfo?)`.
3. Ambos flujos convergen en `_doAuthenticate(...)`.
4. `_doAuthenticate(...)` aplica timeout con `PRINTER_AUTH_TIMEOUT` y evalúa resultado/error.
5. En rechazo por autorización de red (`UnauthorizedError`) se aplica fallback a nuevo auth por botón.

Con el wrapper, quedan registradas entradas/salidas/errores de `connectPrinter` y `reconnectPrinter`, y también de invocaciones `Authorize`/`Authenticate`/`Reauthorize` cuando estas funciones existan en el objeto printer.

## Timeouts relevantes
- `PRINTER_AUTH_TIMEOUT=120000` ms (120 s).
- El timeout se aplica en `_doAuthenticate(...).timeout(PRINTER_AUTH_TIMEOUT)`.

## Mapeo de estados de error
La capa de diagnóstico incorpora `mappedErrorState` cuando detecta:

- `UnauthorizedError`
- `AuthRejectedError`
- `AuthTimedOutError`

Esto permite correlación directa entre error nativo y estado de autenticación observado en runtime.

## Condiciones de fallback

1. **Reauth fallida por `UnauthorizedError` en red**
   - `authFromStored(...)` captura el error.
   - Se elimina auth local (`deauthenticate(...)`) para ese usuario.
   - En `authenticate(...)` se deriva a `requestNewAuth(...)` (nuevo flujo por botón).

2. **Error en autenticación con conexión alternativa disponible**
   - Estado de impresora pasa a `Pending` (se mantiene posibilidad de conexión fallback).

3. **Error sin conexión fallback activa**
   - Estado de impresora pasa a `Unauthenticated`.

4. **Timeout (`ETIMEDOUT`)**
   - Se emite tracking de `Reauthentication Timeout`.
   - El flujo propaga error para manejo superior.

## Operación recomendada
- Filtrar logs por prefijo `[croissant-diagnostic]`.
- Correlacionar por `method` + `durationMs` + `stage`.
- Priorizar eventos `stage=error` con `mappedErrorState` presente.
