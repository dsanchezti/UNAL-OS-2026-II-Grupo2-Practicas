#include <stdio.h>
#include "protocolo.h"
#include "utilidades.h"
#include "interfaz.h"

int leer_solicitud(Solicitud *solicitud) {
    int c;

    printf("\n=== MENU ===\n");
    printf("1. Buscar pais por año\n");
    printf("2. Buscar todos los registros de un pais\n");
    printf("3. Buscar todos los paises de un año\n");
    printf("4. Salir\n");
    printf("Seleccione una opcion: ");

    if (scanf("%d", (int *)&solicitud->operacion) != 1) {
        fprintf(stderr, "Opcion invalida.\n");

        while ((c = getchar()) != '\n' && c != EOF) {}

        return 0;
    }

    while ((c = getchar()) != '\n' && c != EOF) {}

    switch (solicitud->operacion) {
        case BUSCAR_PAIS_ANIO:
            printf("Ingrese el nombre del pais: ");

            if (fgets(solicitud->pais, TAM_PAIS, stdin) == NULL) {
                fprintf(stderr, "No se pudo leer el pais.\n");
                return 0;
            }

            recortar_espacios(solicitud->pais);

            printf("Ingrese el año: ");

            if (scanf("%d", &solicitud->anio) != 1) {
                fprintf(stderr, "Año invalido.\n");

                while ((c = getchar()) != '\n' && c != EOF) {}

                return 0;
            }

            while ((c = getchar()) != '\n' && c != EOF) {}
            break;

        case BUSCAR_PAIS:
            printf("Ingrese el nombre del pais: ");

            if (fgets(solicitud->pais, TAM_PAIS, stdin) == NULL) {
                fprintf(stderr, "No se pudo leer el pais.\n");
                return 0;
            }

            recortar_espacios(solicitud->pais);
            break;

        case BUSCAR_ANIO:
            printf("Ingrese el año: ");

            if (scanf("%d", &solicitud->anio) != 1) {
                fprintf(stderr, "Año invalido.\n");

                while ((c = getchar()) != '\n' && c != EOF) {}

                return 0;
            }

            while ((c = getchar()) != '\n' && c != EOF) {}
            break;

        case SALIR:
            break;

        default:
            printf("Opcion invalida. Intente de nuevo.\n");
            return 0;
    }

    return 1;
}

