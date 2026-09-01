# Cliente-Servidor

Implementación progresiva de comunicación cliente-servidor con sockets TCP, en **Python** y en **C**, desde un intercambio simple de un solo mensaje hasta un servidor multicliente con panel de administración remota.

El proyecto está organizado en versiones incrementales para mostrar la evolución de la solución: cada carpeta agrega un concepto nuevo sobre la anterior (loop de mensajes, múltiples clientes con threading, autenticación y apagado remoto). La misma lógica está implementada dos veces, en dos lenguajes con niveles de abstracción muy distintos, para comparar cómo se resuelven los mismos problemas cuando el lenguaje te da más o menos control sobre los detalles de bajo nivel.

## Estructura del repositorio

```
├── Python/
│   ├── cliente-servidor-single/     # Versión 1: un solo mensaje de ida y vuelta
│   │   ├── servidor.py
│   │   └── cliente.py
│   │
│   ├── Cliente-servidor-loop/       # Versión 2: conversación continua (loop de mensajes)
│   │   ├── servidor-loop.py
│   │   └── cliente-loop.py
│   │
│   └── Multicliente/                # Versión 3: múltiples clientes + admin remoto
│       ├── servidor-multicliente.py
│       ├── cliente-loop.py
│       └── cliente-admin.py
│
└── C/
    ├── Cliente-servidor-single/     # Versión 1 en C
    │   ├── servidor.c
    │   └── cliente.c
    │
    ├── Cliente-servidor-loop/       # Versión 2 en C
    │   ├── servidor-loop.c
    │   └── cliente-loop.c
    │
    └── Multicliente/                # Versión 3 en C (con pthread)
        ├── servidor-multicliente.c
        ├── cliente.c
        └── cliente-adm.c
```

Los binarios compilados no se versionan (ver `.gitignore`); hay que compilar el código fuente localmente antes de correrlo.

---

## Versión Python

### Versión 1 — Intercambio simple (`Python/cliente-servidor-single/`)

El servidor acepta una conexión, recibe un mensaje y responde una sola vez. Es la base para entender `bind`, `listen`, `accept`, `recv` y `sendall`.

```bash
cd Python/cliente-servidor-single
python3 servidor.py      # Terminal 1
python3 cliente.py       # Terminal 2
```

### Versión 2 — Conversación en loop (`Python/Cliente-servidor-loop/`)

El servidor mantiene la conexión abierta y permite mandar varios mensajes seguidos, hasta que el cliente escribe `salir`.

```bash
cd Python/Cliente-servidor-loop
python3 servidor-loop.py    # Terminal 1
python3 cliente-loop.py     # Terminal 2
```

### Versión 3 — Multicliente con panel de administración (`Python/Multicliente/`)

La versión más completa. El servidor atiende múltiples clientes en simultáneo usando un thread por conexión, y permite loguearse como administrador para apagar el servidor de forma remota.

**Funcionalidad:**
- Múltiples clientes conectados a la vez, cada uno en su propio thread.
- Autenticación de admin mediante contraseña (protocolo: primer mensaje `ADMIN:<contraseña>`).
- Un cliente admin autenticado puede chatear normalmente y, en cualquier momento, mandar `shutdown` para apagar el servidor.
- Al apagarse, el servidor avisa a todos los clientes conectados antes de cerrar el proceso.
- La contraseña de admin se lee de una variable de entorno, no está hardcodeada en el código.

```bash
export ADMIN_PASSWORD=tu_contraseña   # opcional, si no se setea usa "admin123"

cd Python/Multicliente
python3 servidor-multicliente.py      # Terminal servidor

python3 cliente-loop.py               # Terminal(es) cliente normal (podés abrir varias)

python3 cliente-admin.py              # Terminal cliente admin
```

Desde el cliente admin, después de ingresar la contraseña, podés chatear normalmente o escribir `shutdown` para apagar el servidor remotamente.

---

## Versión C

Misma funcionalidad que la versión Python, reescrita usando sockets POSIX (`sys/socket.h`) y `pthread` para el manejo de múltiples clientes.

### Compilación

Cada versión se compila por separado con `gcc`:

```bash
gcc -o servidor servidor.c
gcc -o cliente cliente.c
```

La versión Multicliente necesita además linkear la librería de threads:

```bash
gcc -o servidor-multicliente servidor-multicliente.c -lpthread
gcc -o cliente-adm cliente-adm.c
```

### Versión 1 y 2 (`C/Cliente-servidor-single/`, `C/Cliente-servidor-loop/`)

Mismo comportamiento que sus equivalentes en Python. Corré el servidor en una terminal y el cliente en otra, igual que en la versión de Python.

### Versión 3 — Multicliente con admin (`C/Multicliente/`)

```bash
export ADMIN_PASSWORD=tu_contraseña   # opcional, si no se setea usa "admin123"

cd C/Multicliente
gcc -o servidor-multicliente servidor-multicliente.c -lpthread
gcc -o cliente cliente.c
gcc -o cliente-adm cliente-adm.c

./servidor-multicliente     # Terminal servidor
./cliente                   # Terminal(es) cliente normal
./cliente-adm                # Terminal cliente admin
```

---

## Desafíos técnicos resueltos

La parte multicliente con apagado remoto no fue trivial de armar en ninguno de los dos lenguajes — aparecieron varios problemas reales de programación concurrente que vale la pena documentar:

- **Condición de carrera en `accept()`:** un cliente podía colarse justo en el instante en que el servidor empezaba a apagarse, porque no había sincronización entre el hilo que decidía apagar el servidor y el hilo principal esperando conexiones nuevas.

- **Cerrar un socket bloqueado desde otro thread no lo despierta de forma confiable (Python):** en Linux, llamar a `.close()` sobre un socket que está bloqueado en `accept()` en otro thread no garantiza que ese `accept()` se desbloquee. La solución fue usar `socket.settimeout()` para que el loop principal revise periódicamente una bandera de apagado (`threading.Event`) en vez de depender de que el cierre del socket lo interrumpa.

- **El mismo problema, resuelto distinto en C:** en la versión de C se usó `select()` con timeout para el equivalente al `settimeout()` de Python, pero para desbloquear a un cliente ya conectado que está esperando en `read()`, se usó `shutdown(fd, SHUT_RDWR)` en vez de `close()` — a diferencia de `close()`, `shutdown()` le avisa al kernel que la conexión termina, lo cual sí despierta de forma confiable un `read()` bloqueado en otro thread.

- **Threads no-daemon (Python) mantienen vivo el proceso:** aunque la lógica de apagado terminara, el proceso de Python no cerraba hasta que todos los clientes conectados se desconectaran por su cuenta. Se resolvió marcando los threads de clientes como `daemon=True`, y notificando a los clientes activos antes de cerrar el proceso. En C, el comportamiento por defecto de POSIX ya es similar (el proceso termina y mata a los threads activos cuando `main()` retorna), pero se usó `pthread_detach()` para no tener que esperar (`pthread_join`) a cada thread manualmente.

- **Estado compartido entre threads:** la lista de clientes conectados es modificada por múltiples threads a la vez (cuando se conectan y desconectan). Se protegió con un `threading.Lock` en Python y un `pthread_mutex_t` en C, para evitar corromper la lista por accesos concurrentes.

- **Gestión manual de memoria en C:** a diferencia de Python (con garbage collector), cada cliente en C necesita su propia memoria reservada con `malloc()` para pasarle sus datos al thread, y esa memoria se libera explícitamente con `free()` cuando el cliente se desconecta — olvidarse de esto sería un memory leak.

## Tecnologías

- **Python 3** — librería estándar: `socket`, `threading`, `os`
- **C** — `gcc`, sockets POSIX (`sys/socket.h`, `arpa/inet.h`), `pthread`

## Motivación

Este proyecto nació como práctica personal para reforzar conceptos de redes y programación concurrente, en el marco de una formación en programación con interés particular en redes y sistemas. Implementar la misma lógica en dos lenguajes con niveles de abstracción muy distintos ayudó a entender qué resuelve una librería de alto nivel "por debajo" y qué implica manejarlo manualmente.