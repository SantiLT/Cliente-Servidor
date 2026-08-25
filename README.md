# Cliente-Servidor

Implementación progresiva de comunicación cliente-servidor con sockets TCP en Python, desde un intercambio simple de un solo mensaje hasta un servidor multicliente con panel de administración remota.

El proyecto está organizado en versiones incrementales para mostrar la evolución de la solución: cada carpeta agrega un concepto nuevo sobre la anterior (loop de mensajes, múltiples clientes con threading, autenticación y apagado remoto).

## Estructura del repositorio

```
├── cliente-servidor-single/     # Versión 1: un solo mensaje de ida y vuelta
│   ├── servidor.py
│   └── cliente.py
│
├── Cliente-servidor-loop/       # Versión 2: conversación continua (loop de mensajes)
│   ├── servidor-loop.py
│   └── cliente-loop.py
│
└── Multicliente/                # Versión 3: múltiples clientes + admin remoto
    ├── servidor-multicliente.py
    ├── cliente-loop.py
    └── cliente-admin.py
```

## Versión 1 — Intercambio simple (`cliente-servidor-single/`)

El servidor acepta una conexión, recibe un mensaje y responde una sola vez. Es la base para entender `bind`, `listen`, `accept`, `recv` y `sendall`.

**Cómo correrlo:**

Terminal 1:
```bash
cd cliente-servidor-single
python3 servidor.py
```

Terminal 2:
```bash
cd cliente-servidor-single
python3 cliente.py
```

## Versión 2 — Conversación en loop (`Cliente-servidor-loop/`)

El servidor mantiene la conexión abierta y permite mandar varios mensajes seguidos, hasta que el cliente escribe `salir`.

**Cómo correrlo:**

Terminal 1:
```bash
cd Cliente-servidor-loop
python3 servidor-loop.py
```

Terminal 2:
```bash
cd Cliente-servidor-loop
python3 cliente-loop.py
```

## Versión 3 — Multicliente con panel de administración (`Multicliente/`)

La versión más completa. El servidor atiende múltiples clientes en simultáneo usando un thread por conexión, y permite loguearse como administrador para apagar el servidor de forma remota.

**Funcionalidad:**
- Múltiples clientes conectados a la vez, cada uno en su propio thread.
- Autenticación de admin mediante contraseña (protocolo: primer mensaje `ADMIN:<contraseña>`).
- Un cliente admin autenticado puede chatear normalmente y, en cualquier momento, mandar `shutdown` para apagar el servidor.
- Al apagarse, el servidor avisa a todos los clientes conectados antes de cerrar el proceso.
- La contraseña de admin se lee de una variable de entorno, no está hardcodeada en el código.

**Cómo correrlo:**

Antes de arrancar, seteá la contraseña de admin (opcional — si no la seteás, usa `admin123` por defecto):
```bash
export ADMIN_PASSWORD=tu_contraseña
```

Terminal 1 (servidor):
```bash
cd Multicliente
python3 servidor-multicliente.py
```

Terminal 2, 3, etc. (clientes normales, podés abrir varios a la vez):
```bash
cd Multicliente
python3 cliente-loop.py
```

Otra terminal (cliente admin):
```bash
cd Multicliente
python3 cliente-admin.py
```

Desde el cliente admin, después de ingresar la contraseña, podés chatear normalmente o escribir `shutdown` para apagar el servidor remotamente.

## Desafíos técnicos resueltos

Esta versión no fue trivial de armar — en el camino aparecieron varios problemas reales de programación concurrente que vale la pena documentar:

- **Condición de carrera en `accept()`:** un cliente podía colarse justo en el instante en que el servidor empezaba a apagarse, porque no había sincronización entre el hilo que decidía apagar el servidor y el hilo principal esperando conexiones nuevas.

- **Cerrar un socket bloqueado desde otro thread no lo despierta de forma confiable:** en Linux, llamar a `.close()` sobre un socket que está bloqueado en `accept()` en otro thread no garantiza que ese `accept()` se desbloquee. La solución fue usar `socket.settimeout()` para que el loop principal revise periódicamente una bandera de apagado (`threading.Event`) en vez de depender de que el cierre del socket lo interrumpa.

- **Threads no-daemon mantienen vivo el proceso:** aunque la lógica de apagado terminara, el proceso de Python no cerraba hasta que todos los clientes conectados se desconectaran por su cuenta. Se resolvió marcando los threads de clientes como `daemon=True`, y notificando a los clientes activos antes de cerrar el proceso.

- **Estado compartido entre threads:** la lista de clientes conectados es modificada por múltiples threads a la vez (cuando se conectan y desconectan). Se protegió con un `threading.Lock` para evitar corromper la lista por accesos concurrentes.

## Tecnologías

- Python 3
- Librería estándar: `socket`, `threading`, `os`

## Próximos pasos

Reescritura de la versión multicliente en C, usando la librería de sockets POSIX, para comparar el mismo concepto en un lenguaje de más bajo nivel.
