#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#include "memoria_compartida.h"
#include "../common/csv.h"

#define ARCHIVO_CSV "data/population.csv"
#define TAM_LINEA 1024

static int esperar_semáforo(sem_t *sem) {
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }

    return 0;
}


static int enviar_respuesta(
    MemoriaCompartida *memoria,
    sem_t *sem_respuesta,
    sem_t *sem_continuar,
    const Respuesta *respuesta
) {
    memoria->respuesta = *respuesta;

    if (sem_post(sem_respuesta) == -1) {
        perror("Error al publicar respuesta");
        return -1;
    }

    if (esperar_semáforo(sem_continuar) == -1) {
        perror("Error al esperar al cliente");
        return -1;
    }

    return 0;
}

static int enviar_error(
    MemoriaCompartida *memoria,
    sem_t *sem_respuesta,
    sem_t *sem_continuar,
    const char *mensaje
) {
    Respuesta respuesta = {0};

    respuesta.estado = -1;
    respuesta.fin = 1;

    snprintf(
        respuesta.resultado,
        TAM_RESULTADO,
        "%s",
        mensaje
    );

    return enviar_respuesta(
        memoria,
        sem_respuesta,
        sem_continuar,
        &respuesta
    );
}

static int procesar_busqueda(
    MemoriaCompartida *memoria,
    sem_t *sem_respuesta,
    sem_t *sem_continuar,
    const Solicitud *solicitud
) {
    FILE *archivo = fopen(ARCHIVO_CSV, "r");

    if (archivo == NULL) {
        perror("Error al abrir el CSV");

        return enviar_error(
            memoria, sem_respuesta, sem_continuar,
            "No se pudo abrir el archivo CSV."
        );
    }

    char linea[TAM_LINEA];
    int encontrados = 0;

    /* Saltar la cabecera del CSV. */
    if (fgets(linea, sizeof(linea), archivo) == NULL) {
        int hubo_error = ferror(archivo);
        fclose(archivo);

        return enviar_error(
            memoria, sem_respuesta, sem_continuar,
            hubo_error
                ? "No se pudo leer el archivo CSV."
                : "El CSV esta vacio."
        );
    }

    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        char registro[TAM_LINEA];
        snprintf(registro, sizeof(registro), "%s", linea);

        char *cursor = linea;

        char pais_csv[TAM_PAIS];
        char codigo[4];
        char anio_texto[5];
        char valor[11];

        int r1 = leer_campo_csv(
            &cursor, pais_csv, sizeof(pais_csv)
        );
        int r2 = leer_campo_csv(
            &cursor, codigo, sizeof(codigo)
        );
        int r3 = leer_campo_csv(
            &cursor, anio_texto, sizeof(anio_texto)
        );
        int r4 = leer_campo_csv(
            &cursor, valor, sizeof(valor)
        );

        if (r1 != 1 || r2 != 1 || r3 != 1 || r4 != 1) {
            fprintf(stderr, "Fila CSV invalida.\n");
            continue;
        }

        char *fin_anio;
        long anio_leido = strtol(anio_texto, &fin_anio, 10);

        if (*anio_texto == '\0' || *fin_anio != '\0') {
            fprintf(stderr, "Año invalido en el CSV.\n");
            continue;
        }

        int anio_csv = (int)anio_leido;
        int coincide = 0;

        switch (solicitud->operacion) {
            case BUSCAR_PAIS_ANIO:
                coincide =
                    strcasecmp(solicitud->pais, pais_csv) == 0 &&
                    solicitud->anio == anio_csv;
                break;

            case BUSCAR_PAIS:
                coincide =
                    strcasecmp(solicitud->pais, pais_csv) == 0;
                break;

            case BUSCAR_ANIO:
                coincide = solicitud->anio == anio_csv;
                break;

            default:
                fclose(archivo);

                return enviar_error(
                    memoria, sem_respuesta, sem_continuar,
                    "Operacion desconocida."
                );
        }

        if (coincide) {
            Respuesta respuesta = {0};

            respuesta.estado = 0;
            respuesta.fin = 0;

            registro[strcspn(registro, "\r\n")] = '\0';

            snprintf(
                respuesta.resultado,
                TAM_RESULTADO,
                "%.*s",
                TAM_RESULTADO - 1,
                registro
            );

            if (enviar_respuesta(
                    memoria, sem_respuesta,
                    sem_continuar, &respuesta
                ) == -1) {
                fclose(archivo);
                return -1;
            }

            encontrados++;
        }
    }

    if (ferror(archivo)) {
        fclose(archivo);

        return enviar_error(
            memoria, sem_respuesta, sem_continuar,
            "Error durante la lectura del CSV."
        );
    }

    fclose(archivo);

    Respuesta final = {0};
    final.estado = 0;
    final.fin = 1;

    if (encontrados == 0) {
        snprintf(
            final.resultado,
            TAM_RESULTADO,
            "No se encontraron registros coincidentes."
        );
    } else {
        snprintf(
            final.resultado,
            TAM_RESULTADO,
            "Fin de resultados. Total: %d",
            encontrados
        );
    }

    return enviar_respuesta(
        memoria, sem_respuesta, sem_continuar, &final
    );
}

int main(void) {
    int resultado = EXIT_FAILURE;
    int shm_fd = -1;

    MemoriaCompartida *memoria = MAP_FAILED;

    sem_t *sem_solicitud = SEM_FAILED;
    sem_t *sem_respuesta = SEM_FAILED;
    sem_t *sem_continuar = SEM_FAILED;

    shm_unlink(NOMBRE_SHM);
    sem_unlink(SEM_SOLICITUD);
    sem_unlink(SEM_RESPUESTA);
    sem_unlink(SEM_CONTINUAR);

    shm_fd = shm_open(
        NOMBRE_SHM,
        O_CREAT | O_EXCL | O_RDWR,
        0600
    );

    if (shm_fd == -1) {
        perror("Error al crear memoria compartida");
        goto limpieza;
    }

    if (ftruncate(shm_fd, sizeof(MemoriaCompartida)) == -1) {
        perror("Error al definir tamaño de memoria compartida");
        goto limpieza;
    }

    memoria = mmap(
        NULL,
        sizeof(MemoriaCompartida),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0
    );

    if (memoria == MAP_FAILED) {
        perror("Error al mapear memoria compartida");
        goto limpieza;
    }

    memset(memoria, 0, sizeof(MemoriaCompartida));

    sem_solicitud = sem_open(
        SEM_SOLICITUD, O_CREAT | O_EXCL, 0600, 0
    );

    sem_respuesta = sem_open(
        SEM_RESPUESTA, O_CREAT | O_EXCL, 0600, 0
    );

    sem_continuar = sem_open(
        SEM_CONTINUAR, O_CREAT | O_EXCL, 0600, 0
    );

    if (sem_solicitud == SEM_FAILED ||
        sem_respuesta == SEM_FAILED ||
        sem_continuar == SEM_FAILED) {
        perror("Error al crear semaforos");
        goto limpieza;
    }

    printf("Servidor SHM iniciado\n");

    resultado = EXIT_SUCCESS;

    while (1) {
        if (esperar_semáforo(sem_solicitud) == -1) {
            perror("Error al esperar solicitud");
            resultado = EXIT_FAILURE;
            break;
        }

        Solicitud solicitud = memoria->solicitud;

        switch (solicitud.operacion) {
            case BUSCAR_PAIS_ANIO:
            case BUSCAR_PAIS:
            case BUSCAR_ANIO:
                if (procesar_busqueda(
                        memoria,
                        sem_respuesta,
                        sem_continuar,
                        &solicitud
                    ) == -1) {
                    fprintf(stderr, "Error al procesar busqueda.\n");
                    resultado = EXIT_FAILURE;
                    goto limpieza;
                }
                break;

            case SALIR: {
                Respuesta respuesta = {0};

                respuesta.estado = 0;
                respuesta.fin = 1;

                snprintf(
                    respuesta.resultado,
                    TAM_RESULTADO,
                    "Servidor terminando correctamente."
                );

                if (enviar_respuesta(
                        memoria, sem_respuesta,
                        sem_continuar, &respuesta
                    ) == -1) {
                    resultado = EXIT_FAILURE;
                    goto limpieza;
                }

                printf("Solicitud de cierre recibida.\n");
                goto limpieza;
            }

            default:
                if (enviar_error(
                        memoria, sem_respuesta,
                        sem_continuar,
                        "Operacion desconocida."
                    ) == -1) {
                    resultado = EXIT_FAILURE;
                    goto limpieza;
                }
                break;
        }
    }

limpieza:
    if (sem_solicitud != SEM_FAILED)
        sem_close(sem_solicitud);

    if (sem_respuesta != SEM_FAILED)
        sem_close(sem_respuesta);

    if (sem_continuar != SEM_FAILED)
        sem_close(sem_continuar);

    if (memoria != MAP_FAILED)
        munmap(memoria, sizeof(MemoriaCompartida));

    if (shm_fd != -1)
        close(shm_fd);

    shm_unlink(NOMBRE_SHM);
    sem_unlink(SEM_SOLICITUD);
    sem_unlink(SEM_RESPUESTA);
    sem_unlink(SEM_CONTINUAR);

    return resultado;
}
