#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include "../common/protocolo.h"
#include "../common/utilidades.h"
#include "../common/interfaz.h"

#define FIFO_SOLICITUDES "fifo/solicitudes.fifo"
#define FIFO_RESPUESTAS  "fifo/respuestas.fifo"

int main(void) {
    int continuar = 1;
    while (continuar) {
        Solicitud solicitud = {0}; if (!leer_solicitud(&solicitud)) { continue; } if (solicitud.operacion == SALIR) { continuar = 0; }

        int fd_solicitudes = open(
            FIFO_SOLICITUDES,
            O_WRONLY | O_NONBLOCK
        );

        if (fd_solicitudes == -1) {
            if (errno == ENXIO) {
                fprintf(stderr,
                        "Servidor no disponible. "
                        "Inicie el servidor e intente de nuevo.\n");
            } else {
                perror("Error al abrir FIFO de solicitudes");
            }

            continue;
        }
        
        
        ssize_t n = write(fd_solicitudes, &solicitud, sizeof(solicitud));
        close(fd_solicitudes);

        if (n != sizeof(solicitud)) {
            fprintf(stderr, "No se pudo enviar la solicitud completa.\n");
            return 1;
        }

        int fd_respuestas = open(
            FIFO_RESPUESTAS,
            O_RDONLY | O_NONBLOCK
        );
        if (fd_respuestas == -1) {
            perror("Error al abrir FIFO de respuestas");
            return 1;
        }

        Respuesta respuesta;
        int tiempo_agotado = 0;

        do {
            struct pollfd evento = {
                .fd = fd_respuestas,
                .events = POLLIN
            };

            int listo = poll(&evento, 1, 5000);

            if (listo == -1) {
                if (errno == EINTR) {
                    continue;
                }

                perror("Error en poll");
                tiempo_agotado = 1;
                break;
            }

            if (listo == 0) {
                fprintf(stderr,
                        "Tiempo agotado: el servidor no respondio "
                        "en 5 segundos.\n");
                tiempo_agotado = 1;
                break;
            }

            ssize_t n = read(
                fd_respuestas,
                &respuesta,
                sizeof(respuesta)
            );

            if (n != sizeof(respuesta)) {
                fprintf(stderr,
                        "No se recibio una respuesta completa.\n");
                tiempo_agotado = 1;
                break;
            }

            if (respuesta.estado == -1) {
                fprintf(stderr, "Error del servidor: %s\n",
                        respuesta.resultado);
            } else {
                printf("%s\n", respuesta.resultado);
            }

        } while (respuesta.fin == 0);

        close(fd_respuestas);

        if (tiempo_agotado) {
            continue;
        }
    }
    printf("Cliente finalizado.\n");
    return 0;
}