#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "memoria_compartida.h"
#include "../common/interfaz.h"


static int esperar_semaforo_con_tiempo(sem_t *sem) {
    struct timespec limite;

    if (clock_gettime(CLOCK_REALTIME, &limite) == -1) {
        return -1;
    }

    limite.tv_sec += 8;

    while (sem_timedwait(sem, &limite) == -1) {
        if (errno == EINTR) {
            continue;
        }

        if (errno == ETIMEDOUT) {
            fprintf(stderr,
                    "Tiempo agotado: no se recibió respuesta "
                    "del servidor.\n");
        } else {
            perror("Error esperando al servidor");
        }

        return -1;
    }

    return 0;
}

/*static int esperar_semaforo(sem_t *sem) {
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }

    return 0;
}*/

int main(void) {
    int shm_fd = -1;
    int resultado = EXIT_FAILURE;

    MemoriaCompartida *memoria = MAP_FAILED;

    sem_t *sem_solicitud = SEM_FAILED;
    sem_t *sem_respuesta = SEM_FAILED;
    sem_t *sem_continuar = SEM_FAILED;

    shm_fd = shm_open(NOMBRE_SHM, O_RDWR, 0);

    if (shm_fd == -1) {
        perror("Error al abrir memoria compartida");
        fprintf(stderr, "¿El servidor SHM está ejecutándose?\n");
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

    sem_solicitud = sem_open(SEM_SOLICITUD, 0);
    sem_respuesta = sem_open(SEM_RESPUESTA, 0);
    sem_continuar = sem_open(SEM_CONTINUAR, 0);

    if (sem_solicitud == SEM_FAILED ||
        sem_respuesta == SEM_FAILED ||
        sem_continuar == SEM_FAILED) {
        perror("Error al abrir semáforos");
        goto limpieza;
    }

    printf("Cliente SHM conectado al servidor.\n");

    while (1) {
        Solicitud solicitud = {0};


        if (!leer_solicitud(&solicitud)) {
            continue;
        }

        memoria->solicitud = solicitud;

        if (sem_post(sem_solicitud) == -1) {
            perror("Error al notificar solicitud");
            goto limpieza;
        }

        int es_salir = (solicitud.operacion == SALIR);
        int fin = 0;

        while (!fin) {
            if (esperar_semaforo_con_tiempo(sem_respuesta) == -1) {
                goto limpieza;
            }

            Respuesta respuesta = memoria->respuesta;

            if (respuesta.estado == -1) {
                fprintf(
                    stderr,
                    "Error del servidor: %s\n",
                    respuesta.resultado
                );
            } else {
                printf("%s\n", respuesta.resultado);
            }

            fin = respuesta.fin;

            if (sem_post(sem_continuar) == -1) {
                perror("Error al confirmar respuesta");
                goto limpieza;
            }
        }

        if (es_salir) {
            break;
        }
    }

    printf("Cliente SHM finalizado.\n");
    resultado = EXIT_SUCCESS;

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

    return resultado;
}

