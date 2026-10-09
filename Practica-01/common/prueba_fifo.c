#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void) {
    const char *fifo_path = "solicitudes.fifo";
    if (mkfifo(fifo_path, 0666) == -1) {
        perror("Error al crear el FIFO");
        return 1;
    }

    printf("FIFO creado en: %s\n", fifo_path);

    return 0;
}