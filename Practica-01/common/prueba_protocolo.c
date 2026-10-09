
#include <stdio.h>
#include "protocolo.h"

int main(void) {
    Solicitud solicitud;

    solicitud.operacion = BUSCAR;
    snprintf(solicitud.pais, TAM_PAIS, "%s", "Colombia");
    solicitud.anio = 2020;

    printf("Operacion: %d\n", solicitud.operacion);
    printf("Pais: %s\n", solicitud.pais);
    printf("Anio: %d\n", solicitud.anio);

    printf("Tamano de Solicitud: %zu bytes\n", sizeof(Solicitud));
    printf("Tamano de Respuesta: %zu bytes\n", sizeof(Respuesta));

    return 0;
}