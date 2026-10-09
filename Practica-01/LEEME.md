# Práctica 1 — Procesos, llamadas al sistema e IPC en Linux

## 1. Descripción

Este proyecto implementa un sistema cliente-servidor en C para consultar datos de población mediante comunicación entre procesos (IPC) en Linux.

La aplicación cuenta con dos implementaciones de comunicación:

* **FIFO:** utiliza tuberías con nombre para transmitir solicitudes y respuestas.
* **Memoria compartida (SHM):** utiliza una región de memoria compartida y semáforos POSIX para coordinar el acceso entre el cliente y el servidor.

En ambas implementaciones, el cliente presenta un menú de consultas y el servidor procesa las solicitudes recorriendo un archivo CSV. Los dos programas se ejecutan como procesos independientes.

## 2. Base de datos

La aplicación utiliza el conjunto de datos **Population**, publicado en DataHub.

* **Fuente:** [DataHub — Population](https://datahub.io/core/population)
* **Archivo utilizado:** `data/population.csv`
* **Formato:** CSV.

El conjunto contiene cifras de población por país o región y año. El archivo incluye las siguientes columnas:

| Columna        | Descripción              | Ejemplo    |
| -------------- | ------------------------ | ---------- |
| `Country Name` | Nombre del país o región | `Colombia` |
| `Country Code` | Código del país o región | `COL`      |
| `Year`         | Año del registro         | `2020`     |
| `Value`        | Población registrada     | `50629997` |

La primera línea contiene los encabezados y las siguientes contienen los registros. Algunos nombres incluyen comas y aparecen entre comillas, por ejemplo `"Micronesia, Fed. Sts."`. El programa interpreta estos campos como un único valor al procesar el CSV.

El archivo se conserva en disco. Para cada consulta, el servidor lo abre y recorre secuencialmente, comparando los registros con los parámetros recibidos. No se carga la base de datos completa en memoria.

## 3. Requisitos previos

Se necesita un entorno Linux con:

* GCC.
* GNU Make.
* Bibliotecas y encabezados de desarrollo de POSIX.
* El archivo `data/population.csv`.

Los procesos cliente y servidor deben ejecutarse desde la carpeta `Practica-01`, para que las rutas relativas utilizadas por el programa sean correctas.

## 4. Compilación

Desde la carpeta `Practica-01`, ejecutar:

```bash
make
```

El comando compila los ejecutables de ambas implementaciones.

Para compilar solamente la implementación FIFO:

```bash
make fifo
```

Para compilar solamente la implementación de memoria compartida:

```bash
make shm
```

Para eliminar los ejecutables generados:

```bash
make clean
```

## 5. Ejecución con FIFO

El servidor y el cliente se ejecutan en terminales separadas.

**Terminal 1 — Servidor:**

```bash
./fifo/servidor_fifo
```

**Terminal 2 — Cliente:**

```bash
./fifo/cliente_fifo
```

El servidor crea los FIFO necesarios para la comunicación. Durante una ejecución normal, al terminar, intenta eliminar los recursos FIFO que administra.

Si el servidor termina de forma forzada, los FIFO pueden permanecer en el sistema de archivos. En ese caso, se deben verificar y reutilizar correctamente al reiniciar el servidor.

## 6. Ejecución con memoria compartida (SHM)

La implementación SHM también utiliza procesos independientes. El servidor debe iniciarse antes que el cliente, ya que crea y prepara los recursos de comunicación.

**Terminal 1 — Servidor:**

```bash
./shm/servidor_shm
```

**Terminal 2 — Cliente:**

```bash
./shm/cliente_shm
```

El servidor crea la región de memoria compartida y los semáforos POSIX utilizados para coordinar las solicitudes y respuestas. El cliente abre los recursos existentes y los utiliza para intercambiar información con el servidor.

Al terminar normalmente, el servidor libera los recursos que administra, incluyendo el mapeo de memoria, los descriptores y los nombres de los objetos de memoria compartida y semáforos.

Si un proceso termina de forma inesperada, algunos recursos con nombre pueden permanecer. Su estado debe verificarse antes de iniciar otra ejecución.

## 7. Menú y formato de entrada

Ambas implementaciones ofrecen el mismo menú:

1. Buscar un país para un año específico.
2. Buscar todos los registros de un país.
3. Buscar todos los países de un año.
4. Salir.

### Opción 1: buscar país por año

Se solicita el nombre del país y el año.

```text
Seleccione una opcion: 1
Ingrese el nombre del pais: Colombia
Ingrese el año: 2020
```

### Opción 2: buscar todos los registros de un país

Se solicita el nombre del país.

```text
Seleccione una opcion: 2
Ingrese el nombre del pais: Colombia
```

### Opción 3: buscar todos los países de un año

Se solicita el año.

```text
Seleccione una opcion: 3
Ingrese el año: 2020
```

### Opción 4: salir

```text
Seleccione una opcion: 4
```

El cliente envía la solicitud de terminación al servidor.

### Formato de entrada esperado

* Las opciones del menú deben ingresarse como números enteros del 1 al 4.
* El año debe ingresarse como un número entero.
* El país debe ingresarse mediante su nombre, tal como aparece en la columna `Country Name` del CSV.
* Se eliminan los espacios en blanco al inicio y al final del nombre ingresado.
* La comparación de nombres no distingue entre mayúsculas y minúsculas.
* Los nombres que contienen comas deben escribirse completos, por ejemplo `Micronesia, Fed. Sts.`.

Si una consulta no encuentra coincidencias, el cliente informa que no hay resultados.

## 8. Protocolo de comunicación

El protocolo común está definido en `common/protocolo.h` y es utilizado por ambas implementaciones.

### Estructura de solicitud

La estructura `Solicitud` contiene:

* `operacion`: identifica la consulta solicitada o la orden de salida.
* `pais`: nombre del país o región, cuando la operación lo requiere.
* `anio`: año de consulta, cuando la operación lo requiere.

Las operaciones se identifican mediante los valores del enumerado `Operacion`:

* `BUSCAR_PAIS_ANIO`
* `BUSCAR_PAIS`
* `BUSCAR_ANIO`
* `SALIR`

### Estructura de respuesta

La estructura `Respuesta` contiene:

* `estado`: indica si la respuesta corresponde a una operación exitosa o a un error.
* `fin`: indica si la respuesta es la última de la consulta.
* `resultado`: contiene el registro, el mensaje de estado o la descripción del error.

Para una búsqueda, el servidor puede enviar varias respuestas con registros y termina la consulta con una respuesta final. Esto permite distinguir entre los resultados intermedios y la finalización de la operación.

La orden `SALIR` solicita al servidor terminar su ejecución.

## 9. Comunicación mediante FIFO

La implementación FIFO utiliza dos tuberías con nombre:

| Recurso                 | Función                                             |
| ----------------------- | --------------------------------------------------- |
| `fifo/solicitudes.fifo` | El cliente envía solicitudes al servidor.           |
| `fifo/respuestas.fifo`  | El servidor envía resultados y mensajes al cliente. |

Las operaciones principales utilizadas son:

* `mkfifo()`: crea las tuberías con nombre.
* `open()`: abre las tuberías para lectura o escritura.
* `read()`: recibe solicitudes o respuestas.
* `write()`: transmite solicitudes o respuestas.
* `close()`: cierra los descriptores.
* `unlink()`: elimina los nombres de los FIFO al realizar la limpieza.

El cliente utiliza `O_NONBLOCK` al abrir el FIFO de solicitudes para detectar cuando no hay un lector disponible, en lugar de bloquearse indefinidamente en esa apertura.

En la recepción de respuestas, se utiliza `poll()` con un tiempo límite de cinco segundos para detectar la ausencia de datos dentro del intervalo configurado. Si se agota el tiempo, el cliente informa el problema y regresa al menú.

Este límite permite detectar la falta de respuesta, aunque no demuestra por sí solo que el servidor haya terminado: también podría estar ocupado o tardar más de lo esperado.

## 10. Comunicación mediante memoria compartida (SHM)

La implementación SHM utiliza una región de memoria compartida para intercambiar solicitudes y respuestas, junto con semáforos POSIX para coordinar el acceso.

### Memoria compartida

La estructura `MemoriaCompartida`, definida en `shm/memoria_compartida.h`, contiene los campos necesarios para compartir una `Solicitud` y una `Respuesta`.

Los recursos principales son:

* `shm_open()`: crea o abre el objeto de memoria compartida.
* `ftruncate()`: establece el tamaño de la región compartida.
* `mmap()`: mapea la región en el espacio de direcciones del proceso.
* `munmap()`: elimina el mapeo al finalizar.
* `shm_unlink()`: elimina el nombre del objeto de memoria compartida.

La memoria compartida evita tener que copiar cada mensaje a través de una tubería: ambos procesos acceden a una misma región de memoria. Sin embargo, es necesario sincronizar los accesos para evitar que un proceso lea datos incompletos o sobrescriba información que el otro todavía necesita.

### Sincronización con semáforos POSIX

La implementación utiliza semáforos con nombre para coordinar el intercambio entre cliente y servidor:

* `sem_solicitud`: notifica al servidor que hay una solicitud disponible.
* `sem_respuesta`: notifica al cliente que hay una respuesta disponible.
* `sem_continuar`: permite coordinar la continuación del intercambio después de que el cliente consume una respuesta.

Las operaciones principales utilizadas son:

* `sem_open()`: crea o abre un semáforo.
* `sem_wait()`: espera a que el semáforo permita continuar.
* `sem_post()`: notifica que una etapa de la comunicación ha terminado.
* `sem_close()`: cierra el semáforo en el proceso.
* `sem_unlink()`: elimina el nombre del semáforo cuando corresponde.

El uso de semáforos evita tener que consultar repetidamente el estado de la solicitud mediante espera activa (*busy waiting*). Los procesos pueden bloquearse hasta recibir la notificación correspondiente.

El cliente y el servidor siguen un protocolo de solicitud, respuesta y confirmación para coordinar el intercambio de resultados.

## 11. Manejo de errores y recursos

Ambas implementaciones contemplan situaciones de error durante la comunicación, como solicitudes o respuestas incompletas y fallos en operaciones del sistema.

### FIFO

* Se comprueba el resultado de las operaciones de apertura, lectura y escritura.
* El cliente detecta si no hay un lector disponible al abrir el FIFO de solicitudes.
* Se utiliza un límite de espera para la recepción de respuestas.
* Los FIFO se eliminan durante la terminación normal del servidor.

### SHM

* Se comprueban las operaciones de apertura y creación de memoria compartida y semáforos.
* Los semáforos coordinan el intercambio de solicitudes y respuestas.
* Durante una terminación normal, se cierran los recursos abiertos y se eliminan los objetos que administra el servidor.

La limpieza normal no está garantizada cuando un proceso termina de forma forzada. Por ello, se debe tener en cuenta la posible existencia de recursos persistentes antes de iniciar otra ejecución.

## 12. Estructura del proyecto

```text
Practica-01/
├── common/
│   ├── protocolo.h
│   ├── utilidades.h
│   ├── utilidades.c
│   ├── interfaz.h
│   ├── interfaz.c
│   ├── csv.h
│   └── csv.c
├── data/
│   └── population.csv
├── fifo/
│   ├── cliente_fifo.c
│   └── servidor_fifo.c
├── shm/
│   ├── cliente_shm.c
│   ├── servidor_shm.c
│   └── memoria_compartida.h
├── Makefile
└── LEEME.md
```
## 13 Observaciones realizadas con herramientas de Linux

### 1. Identificación de procesos mediante `ps` Con cliente y servidor FIFO

Se ejecutó el comando `ps -eo pid,ppid,stat,comm,args | grep -E 'servidor_fifo|cliente_fifo'` para observar los procesos relacionados con la implementación mediante FIFO.

|   PID |  PPID | Estado (`STAT`) | Comando         | Descripción                                               |
| ----: | ----: | :-------------: | --------------- | --------------------------------------------------------- |
| 78009 | 66213 |       `S+`      | `servidor_fifo` | Proceso del servidor FIFO en ejecución.                   |
| 78047 | 66195 |       `S+`      | `cliente_fifo`  | Proceso del cliente FIFO en ejecución.                    |
| 78144 | 36683 |       `S+`      | `grep`          | Proceso temporal que realizó la búsqueda de los procesos. |

**Análisis de los resultados:**

* **PID:** identifica cada proceso mientras está activo.
* **PPID:** indica el identificador del proceso padre. El cliente y el servidor tienen PPID diferentes, por lo que no comparten el mismo proceso padre directo.
* **Estado `S+`:** la letra `S` indica que el proceso está en estado de sueño interrumpible, es decir, puede estar esperando un evento, una operación de entrada/salida o una señal. El signo `+` indica que pertenece al grupo de procesos en primer plano de su terminal.
* Se observa que el cliente y el servidor FIFO se ejecutan como procesos independientes, lo que permite comprobar que la comunicación se realiza entre procesos separados.

### 2. Recursos de memoria compartida y semáforos con `ls`

Se ejecutó el comando `ls -l /dev/shm` mientras el servidor de memoria compartida estaba en ejecución tras solicitar la busqueda de entradas con país `Colombia`, con el objetivo de observar los recursos IPC creados por el programa.

| Permisos     | Propietario | Tamaño (bytes) | Recurso             | Función                                                                                         |
| ------------ | ----------- | -------------: | ------------------- | ----------------------------------------------------------------------------------------------- |
| `-rw-------` | danni       |            628 | `population_shm`    | Objeto de memoria compartida que almacena la solicitud del cliente y la respuesta del servidor. |
| `-rw-------` | danni       |             32 | `sem.sem_continuar` | Sincroniza la continuación del servidor después de que el cliente recibe una respuesta.         |
| `-rw-------` | danni       |             32 | `sem.sem_respuesta` | Notifica al cliente que hay una respuesta disponible.                                           |
| `-rw-------` | danni       |             32 | `sem.sem_solicitud` | Notifica al servidor que hay una solicitud disponible.                                          |


* Se encontró el objeto `population_shm`, correspondiente al nombre `/population_shm` definido en `memoria_compartida.h`. Su tamaño observado es de 628 bytes.
* Se encontraron los tres semáforos POSIX utilizados por el programa. Linux los muestra con el prefijo `sem.` en `/dev/shm`.
* Los permisos `-rw-------` indican que únicamente el propietario tiene permisos de lectura y escritura sobre estos recursos.
* La presencia de estos cuatro recursos evidencia que la implementación utiliza una región de memoria compartida para intercambiar datos y tres semáforos para coordinar el acceso entre el cliente y el servidor.


### 3. Inspección de llamadas al sistema mediante `strace` del cliente FIFO


Se utilizó la herramienta `strace` para observar las llamadas al sistema realizadas por el cliente durante una búsqueda de todos los registros correspondientes a `Colombia`. Posteriormente, se filtró el archivo de trazas mediante el comando `grep -E 'openat|read|write|poll|close' /tmp/cliente_fifo.trace`.

| Llamada al sistema | Evidencia observada                                          | Función dentro del programa                                                                           |
| ------------------ | ------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------- |
| `openat()`         | Se abrió `fifo/solicitudes.fifo` con `O_WRONLY\|O_NONBLOCK`. | Abre el FIFO de solicitudes en modo de escritura sin esperar indefinidamente a que exista un lector.  |
| `write()`          | Se escribieron 108 bytes en el descriptor `3`.               | Envía la solicitud del cliente al servidor, incluyendo la operación de búsqueda y el nombre del país. |
| `close()`          | Se cerró el descriptor `3` después de enviar la solicitud.   | Libera el descriptor del FIFO de solicitudes.                                                         |
| `openat()`         | Se abrió `fifo/respuestas.fifo` con `O_RDONLY\|O_NONBLOCK`.  | Abre el FIFO utilizado para recibir los resultados del servidor.                                      |
| `poll()`           | La llamada devolvió `1` y mostró `POLLIN`.                   | Indica que hay datos disponibles para leer en el FIFO de respuestas.                                  |
| `read()`           | Se leyeron respuestas de 520 bytes.                          | Recibe las estructuras `Respuesta` enviadas por el servidor.                                          |
| `write()`          | Se mostraron resultados como `Colombia,COL,1960,15606209`.   | Escribe los resultados recibidos en la salida de la terminal.                                         |
| `close()`          | Se cerró el descriptor del FIFO de respuestas.               | Libera el recurso después de terminar de recibir los resultados.                                      |



* Se comprobó que el cliente utiliza llamadas al sistema para abrir, leer, escribir y cerrar los FIFO.
* La solicitud enviada ocupa 108 bytes, mientras que cada respuesta ocupa 520 bytes, de acuerdo con los tamaños de las estructuras utilizadas por el protocolo.
* La llamada `poll()` permite esperar hasta cinco segundos por datos disponibles, en lugar de consultar continuamente el FIFO mediante un ciclo de espera activa.
* La búsqueda de Colombia devolvió registros desde 1960 hasta 2025 y el servidor informó `Fin de resultados. Total: 66`.
* Después de recibir la respuesta final, el cliente cerró el FIFO de respuestas y volvió a mostrar el menú.

