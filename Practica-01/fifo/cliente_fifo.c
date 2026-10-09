#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "../common/protocolo.h"
#include "../common/utilidades.h"

#define FIFO_SOLICITUDES "fifo/solicitudes.fifo"
#define FIFO_RESPUESTAS  "fifo/respuestas.fifo"

int main(void) {
    int continuar = 1;
    while (continuar) {
        Solicitud solicitud = {0};

        printf("\n=== MENU ===\n");
        printf("1. Buscar pais por año\n");
        printf("2. Buscar todos los registros de un pais\n");
        printf("3. Buscar todos los paises de un año\n");
        printf("4. Salir\n");
        printf("Seleccione una opcion: ");

        if (scanf("%d", (int *)&solicitud.operacion) != 1) {
            fprintf(stderr, "Opcion invalida.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }

        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        switch (solicitud.operacion) {
            case BUSCAR_PAIS_ANIO:
                printf("Ingrese el nombre del pais: ");
                if (fgets(solicitud.pais, TAM_PAIS, stdin) == NULL) {
                    fprintf(stderr, "No se pudo leer el pais.\n");
                    continue;
                }
                recortar_espacios(solicitud.pais);

                printf("Ingrese el año: ");
                if (scanf("%d", &solicitud.anio) != 1) {
                    fprintf(stderr, "Año invalido.\n");
                    while ((c = getchar()) != '\n' && c != EOF) {}
                    continue;
                }
                while ((c = getchar()) != '\n' && c != EOF) {}
                break;

            case BUSCAR_PAIS:
                printf("Ingrese el nombre del pais: ");
                if (fgets(solicitud.pais, TAM_PAIS, stdin) == NULL) {
                    fprintf(stderr, "No se pudo leer el pais.\n");
                    continue;
                }
                recortar_espacios(solicitud.pais);
                break;

            case BUSCAR_ANIO:
                printf("Ingrese el año: ");
                if (scanf("%d", &solicitud.anio) != 1) {
                    fprintf(stderr, "Año invalido.\n");
                    while ((c = getchar()) != '\n' && c != EOF) {}
                    continue;
                }
                while ((c = getchar()) != '\n' && c != EOF) {}
                break;

            case SALIR:
                continuar = 0;
                break;

            default:
                printf("Opcion invalida. Intente de nuevo.\n");
                continue;
        }

        int fd_solicitudes = open(FIFO_SOLICITUDES, O_WRONLY);
        if (fd_solicitudes == -1) {
            perror("Error al abrir FIFO de solicitudes");
            return 1;
        }

        ssize_t n = write(fd_solicitudes, &solicitud, sizeof(solicitud));
        close(fd_solicitudes);

        if (n != sizeof(solicitud)) {
            fprintf(stderr, "No se pudo enviar la solicitud completa.\n");
            return 1;
        }

        int fd_respuestas = open(FIFO_RESPUESTAS, O_RDONLY);
        if (fd_respuestas == -1) {
            perror("Error al abrir FIFO de respuestas");
            return 1;
        }

        Respuesta respuesta;

        do {
            n = read(fd_respuestas, &respuesta, sizeof(respuesta));

            if (n != sizeof(respuesta)) {
                fprintf(stderr, "No se recibio una respuesta completa.\n");
                close(fd_respuestas);
                return 1;
            }

            if (respuesta.estado == -1) {
                fprintf(stderr, "Error del servidor: %s\n",
                        respuesta.resultado);
            }else{
                printf("%s\n", respuesta.resultado);
            }

        } while (respuesta.fin == 0);

        close(fd_respuestas);
    }

    printf("Cliente finalizado.\n");
    return 0;
}