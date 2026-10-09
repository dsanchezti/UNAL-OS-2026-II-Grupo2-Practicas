#include "utilidades.h"
#include <ctype.h>
#include <string.h>

void recortar_espacios(char *texto) {
char *inicio = texto;
    while (isspace((unsigned char)*inicio)) {
        inicio++;
    }

    if (inicio != texto) {
        memmove(texto, inicio, strlen(inicio) + 1);
    }

    size_t longitud = strlen(texto);

    while (longitud > 0 &&
        isspace((unsigned char)texto[longitud - 1])) {
        texto[--longitud] = '\0';
    }
}
