
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "../common/protocolo.h"
#include <strings.h>
#include <ctype.h>
#define FIFO_SOLICITUDES "fifo/solicitudes.fifo"
#define FIFO_RESPUESTAS  "fifo/respuestas.fifo"

int crear_fifo(const char *ruta) {
    if (mkfifo(ruta, 0660) == -1) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return -1;
        }
    }

    return 0;
}


static int leer_campo_csv(char **cursor, char *destino, size_t capacidad) {
    char *p = *cursor;
    size_t i = 0;

    if (*p == '\0' || *p == '\n' || *p == '\r') {
        return 0;
    }

    if (*p == '"') {
        p++;

        while (*p != '\0') {
            if (*p == '"') {
                if (p[1] == '"') {
                    if (i + 1 < capacidad) {
                        destino[i++] = '"';
                    }
                    p += 2;
                } else {
                    p++;
                    break;
                }
            } else {
                if (i + 1 < capacidad) {
                    destino[i++] = *p;
                }
                p++;
            }
        }

        while (*p != '\0' && *p != ',' &&
               *p != '\n' && *p != '\r') {
            p++;
        }
    } else {
        while (*p != '\0' && *p != ',' &&
               *p != '\n' && *p != '\r') {
            if (i + 1 < capacidad) {
                destino[i++] = *p;
            }
            p++;
        }
    }

    destino[i] = '\0';

    if (*p == ',') {
        p++;
    }

    *cursor = p;
    return 1;
}


#define ARCHIVO_CSV "data/population.csv"
#define TAM_LINEA 1024

int procesar_busqueda(int fd_respuestas, const Solicitud *solicitud) {
    FILE *archivo = fopen(ARCHIVO_CSV, "r");

    if (archivo == NULL) {
        perror("Error al abrir el CSV");
        
        Respuesta respuesta = {0};
        respuesta.estado = -1;
        respuesta.fin = 1;

        snprintf(respuesta.resultado, TAM_RESULTADO,
                 "No se pudo abrir el archivo CSV.");

        return write(fd_respuestas, &respuesta,
                     sizeof(respuesta)) == sizeof(respuesta) ? 0 : -1;
    }

    char linea[TAM_LINEA];
    int encontrados = 0;

    if (fgets(linea, sizeof(linea), archivo) == NULL) {
        fclose(archivo);

        Respuesta respuesta = {0};
        respuesta.estado = -1;
        respuesta.fin = 1;

        snprintf(respuesta.resultado, TAM_RESULTADO,
                 "El CSV esta vacio o no se pudo leer.");

        return write(fd_respuestas, &respuesta,
                     sizeof(respuesta)) == sizeof(respuesta) ? 0 : -1;
    }

    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        char registro[TAM_LINEA];
        snprintf(registro, sizeof(registro), "%s", linea);

        char *cursor = linea;

        char pais_csv[TAM_PAIS];  // 100 bytes
        char codigo[4];           // 3 bytes + '\0'
        char anio_texto[5];       // 4 bytes + '\0'
        char valor[11];           // 10 bytes + '\0'

        int r1 = leer_campo_csv(&cursor, pais_csv, sizeof(pais_csv));
        int r2 = leer_campo_csv(&cursor, codigo, sizeof(codigo));
        int r3 = leer_campo_csv(&cursor, anio_texto, sizeof(anio_texto));
        int r4 = leer_campo_csv(&cursor, valor, sizeof(valor));

        if (r1 != 1 || r2 != 1 || r3 != 1 || r4 != 1) {
            fprintf(stderr, "Se encontro una fila CSV invalida.\n");
            continue;
        }

        char *fin_anio;
        long anio_leido = strtol(anio_texto, &fin_anio, 10);

        if (*anio_texto == '\0' || *fin_anio != '\0') {
            fprintf(stderr, "Año invalido en el CSV: %s\n", anio_texto);
            continue;
        }

        int anio_csv = (int)anio_leido;
        int coincide = 0;

        /* Seleccionar el criterio de búsqueda. */
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
                return -1;
        }

        if (coincide) {
            Respuesta respuesta = {0};

            respuesta.estado = 0;
            respuesta.fin = 0;

            registro[strcspn(registro, "\r\n")] = '\0';

            snprintf(respuesta.resultado, TAM_RESULTADO,
                    "%.*s", TAM_RESULTADO - 1, registro);

            ssize_t n = write(fd_respuestas, &respuesta,
                              sizeof(respuesta));

            if (n != sizeof(respuesta)) {
                perror("Error al enviar resultado");
                fclose(archivo);
                return -1;
            }

            encontrados++;
        }
    }

    if (ferror(archivo)) {
        fclose(archivo);

        Respuesta respuesta = {0};
        respuesta.estado = -1;
        respuesta.fin = 1;

        snprintf(respuesta.resultado, TAM_RESULTADO,
                 "Error durante la lectura del CSV.");

        return write(fd_respuestas, &respuesta,
                     sizeof(respuesta)) == sizeof(respuesta) ? 0 : -1;
    }

    fclose(archivo);

    Respuesta final = {0};
    final.estado = 0;
    final.fin = 1;

    if (encontrados == 0) {
        snprintf(final.resultado, TAM_RESULTADO,
                 "No se encontraron registros coincidentes.");
    } else {
        snprintf(final.resultado, TAM_RESULTADO,
                 "Fin de resultados. Total: %d", encontrados);
    }

    ssize_t n = write(fd_respuestas, &final, sizeof(final));

    if (n != sizeof(final)) {
        perror("Error al enviar respuesta final");
        return -1;
    }

    return 0;
}


int main(void) {
    if (crear_fifo(FIFO_SOLICITUDES) == -1 ||
        crear_fifo(FIFO_RESPUESTAS) == -1) {
        return 1;
    }

    printf("Servidor FIFO iniciado.\n");

    int continuar = 1;

    while (continuar) {
        int fd_solicitudes = open(FIFO_SOLICITUDES, O_RDONLY);

        if (fd_solicitudes == -1) {
            perror("Error al abrir FIFO de solicitudes");
            break;
        }

        Solicitud solicitud = {0};

        ssize_t n = read(
            fd_solicitudes,
            &solicitud,
            sizeof(solicitud)
        );

        close(fd_solicitudes);

        if (n != sizeof(solicitud)) {
            fprintf(stderr,
                    "No se recibio la solicitud completa.\n");
            continue;
        }

        int fd_respuestas = open(FIFO_RESPUESTAS, O_WRONLY);

        if (fd_respuestas == -1) {
            perror("Error al abrir FIFO de respuestas");
            break;
        }

        switch (solicitud.operacion) {
            case BUSCAR_PAIS_ANIO:
            case BUSCAR_PAIS:
            case BUSCAR_ANIO:
                if (procesar_busqueda(fd_respuestas, &solicitud) == -1) {
                    fprintf(stderr,
                            "Error al procesar la busqueda.\n");
                    continuar = 0;
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

                n = write(
                    fd_respuestas,
                    &respuesta,
                    sizeof(respuesta)
                );

                if (n != sizeof(respuesta)) {
                    fprintf(stderr,
                            "No se pudo enviar la confirmacion de cierre.\n");
                    continuar = 0;
                } else {
                    continuar = 0;
                }

                break;
            }

            default: {
                Respuesta respuesta = {0};

                respuesta.estado = -1;
                respuesta.fin = 1;

                snprintf(
                    respuesta.resultado,
                    TAM_RESULTADO,
                    "Operacion desconocida."
                );

                n = write(
                    fd_respuestas,
                    &respuesta,
                    sizeof(respuesta)
                );

                if (n != sizeof(respuesta)) {
                    fprintf(stderr,
                            "No se pudo enviar el mensaje de error.\n");
                    continuar = 0;
                }

                break;
            }
        }

        close(fd_respuestas);
    }

    if (unlink(FIFO_SOLICITUDES) == -1) {
        perror("Error al eliminar FIFO de solicitudes");
    }

    if (unlink(FIFO_RESPUESTAS) == -1) {
        perror("Error al eliminar FIFO de respuestas");
    }

    printf("Servidor finalizado.\n");

    return 0;
}