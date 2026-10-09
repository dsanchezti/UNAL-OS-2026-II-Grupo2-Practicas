#include "csv.h"

int leer_campo_csv(char **cursor, char *destino, size_t capacidad) {
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