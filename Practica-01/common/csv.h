#ifndef CSV_H
#define CSV_H
#include <stddef.h>

int leer_campo_csv(
    char **cursor,
    char *destino,
    size_t capacidad
);

#endif