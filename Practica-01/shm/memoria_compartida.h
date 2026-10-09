#ifndef MEMORIA_COMPARTIDA_H
#define MEMORIA_COMPARTIDA_H

#include "../common/protocolo.h"

#define NOMBRE_SHM "/population_shm"
#define SEM_SOLICITUD "/sem_solicitud"
#define SEM_RESPUESTA "/sem_respuesta"
#define SEM_CONTINUAR "/sem_continuar"

typedef struct {
    Solicitud solicitud;
    Respuesta respuesta;
} MemoriaCompartida;

#endif