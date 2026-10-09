# Práctica 1 — Procesos, llamadas al sistema e IPC en Linux

## 1. Descripción

Este proyecto implementa un sistema cliente-servidor en C para consultar datos de población mediante comunicación entre procesos (IPC) usando FIFO con nombre en Linux.

El cliente permite realizar consultas sobre una base de datos CSV. El servidor recibe las solicitudes, recorre el archivo y envía los registros coincidentes al cliente.


## 2. Base de datos

La aplicación utiliza el conjunto de datos **Population**, publicado en DataHub:

* **Fuente:** [DataHub — Population](https://datahub.io/core/population)
* **Archivo utilizado:** `data/population.csv`
* **Formato:** CSV.


El conjunto contiene cifras de población por país o región y año. El archivo CSV incluye las siguientes columnas:

| Columna        | Descripción              | Ejemplo    |
| -------------- | ------------------------ | ---------- |
| `Country Name` | Nombre del país o región | `Colombia` |
| `Country Code` | Código del país o región | `COL`      |
| `Year`         | Año del registro         | `2020`     |
| `Value`        | Población registrada     | `50629997` |

La primera línea contiene los encabezados y las siguientes contienen los registros. Algunos nombres incluyen comas y aparecen entre comillas, por ejemplo `"Micronesia, Fed. Sts."`. El servidor interpreta estos campos como un único valor al procesar el CSV.

El archivo se conserva en disco. Para cada consulta, el servidor lo abre y recorre secuencialmente, comparando los registros con los parámetros recibidos del cliente. No se carga la base de datos completa en memoria.



## 5. Compilación

Desde la carpeta `Practica-01`, ejecutar:

```bash
make
```

Para eliminar los ejecutables generados:

```bash
make clean
```

## 5. Ejecución (FIFO)

El servidor debe iniciarse antes que el cliente. Los comandos se ejecutan desde la carpeta `Practica-01`.

En una primera terminal:

```bash
./fifo/servidor_fifo
```

En una segunda terminal:

```bash
./fifo/cliente_fifo
```

El servidor crea los FIFO necesarios para la comunicación. Al terminar, elimina los FIFO que administra.

## 6. Menú y formato de entrada

El cliente presenta un menú con las siguientes opciones:

1. Buscar un país para un año específico.
2. Buscar todos los registros de un país.
3. Buscar todos los países de un año.
4. Salir.

### Opción 1: buscar país por año

Se solicita el nombre del país y el año.

Ejemplo:

```text
Seleccione una opcion: 1
Ingrese el nombre del pais: Colombia
Ingrese el año: 2020
```

### Opción 2: buscar todos los registros de un país

Se solicita el nombre del país.

Ejemplo:

```text
Seleccione una opcion: 2
Ingrese el nombre del pais: Colombia
```

La consulta devuelve los registros disponibles para ese país.

### Opción 3: buscar todos los países de un año

Se solicita el año.

Ejemplo:

```text
Seleccione una opcion: 3
Ingrese el año: 2020
```

La consulta devuelve los registros disponibles para ese año.

### Opción 4: salir

```text
Seleccione una opcion: 4
```

Solicita al servidor terminar su ejecución.

### Formato de entrada esperado

* Las opciones del menú deben ingresarse como números enteros del 1 al 4.
* El año debe ingresarse como un número entero.
* El país debe ingresarse mediante su nombre, tal como aparece en la columna `Country Name` del CSV.
* Se eliminan los espacios en blanco al inicio y al final del nombre ingresado.
* La comparación de nombres no distingue entre mayúsculas y minúsculas.
* Los nombres que contienen comas deben escribirse completos, por ejemplo `Micronesia, Fed. Sts.`.

Si una consulta no encuentra coincidencias, el cliente informa que no hay resultados.

## 7. Comunicación por FIFO

La comunicación utiliza dos FIFO con nombre:

* `fifo/solicitudes.fifo`: el cliente envía solicitudes al servidor.
* `fifo/respuestas.fifo`: el servidor envía resultados y mensajes al cliente.

El protocolo compartido se define en `common/protocolo.h`.

La estructura `Solicitud` contiene la operación, el país y el año. La estructura `Respuesta` contiene el estado de la operación, el indicador de finalización y el texto del resultado.

El servidor procesa una solicitud, envía los registros coincidentes y termina cada consulta con una respuesta final. La operación de salida solicita la terminación del servidor.

## 8. Estructura del proyecto

```text
Practica-01/
├── common/
│   ├── protocolo.h
│   ├── utilidades.h
│   └── utilidades.c
├── data/
│   └── population.csv
├── fifo/
│   ├── cliente_fifo.c
│   └── servidor_fifo.c
├── shm/
├── Makefile
└── LEEME.md
```
