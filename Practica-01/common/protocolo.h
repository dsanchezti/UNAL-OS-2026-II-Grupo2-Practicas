#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#define TAM_PAIS 100
#define TAM_RESULTADO 512

typedef enum {
    BUSCAR_PAIS_ANIO = 1,
    BUSCAR_PAIS = 2,
    BUSCAR_ANIO = 3,
    SALIR = 4
} Operacion;

typedef struct {
    Operacion operacion;
    char pais[TAM_PAIS];
    int anio;
} Solicitud;

typedef struct {
    int estado;
    int fin;
    char resultado[TAM_RESULTADO];
} Respuesta;

#endif