#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h> // Necesario para O_CREAT y O_EXCL
#include <time.h>
#include <unistd.h>
#include "datosCompartidos.h" // Estructura
#include "Constantes.h"

void printDatos(struct datosCompartida *d);

int main(int argc, char *argv[])
{

    // Valores ingresados
    char *ID;
    int clave;
    int numeroEspacio;

    // Verificar que se hayan ingresado los 3 argumentos
    if (argc != 4)
    {
        printf("Uso: programa <string1> <numero1> <numero2>\n");
        return 1;
    }

    // Obtener los argumentos y convertir el número a entero
    ID = argv[1];
    clave = atoi(argv[2]);
    numeroEspacio = atoi(argv[3]);

    // Para correr la vara varias veces borramos los semaforos
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");

    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;

    // Semáforo que contiene el num de espacios ocupados del buffer
    // Con O_EXCL se asegura que si el semáforo ya existe, entonces la función retorna error
    sem_llenos = sem_open("/sem_llenos", O_CREAT | O_EXCL, 0644, 0);
    if (sem_llenos == SEM_FAILED)
    {
        perror("Error abriendo el semáforo de espacios ocupados");
        exit(1);
    }
    int value;
    sem_getvalue(sem_llenos, &value);
    printf("sem_llenos value: %d\n", value);

    // Semáforo que contiene el num de espacios vacíos del buffer
    // 0644 son los permisos. 6->owner R/W  4->the group read-only 4->all others read-only
    sem_vacios = sem_open("/sem_vacios", O_CREAT | O_EXCL, 0644, numeroEspacio);
    if (sem_vacios == SEM_FAILED)
    {
        perror("Error abriendo el semáforo de espacios vacíos");
        exit(1);
    }
    sem_getvalue(sem_vacios, &value);
    printf("sem_vacios value: %d\n", value);

    // Inicializamos la estructura compartida
    struct datosCompartida *datos;

    // Crear una clave única para la memoria compartida
    key_t key = ftok(KEY_PATH, *ID);
    printf("key: %-20d\n", key);

    size_t tamano = sizeof(struct datosCompartida);
    printf("tamaño mem compartida: %zu bytes\n", tamano);
    // Crear la memoria compartida
    int shmid = shmget(key, tamano, 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("Error creando la memoria compartida");
        exit(1);
    }
    printf("ID mem compartida: %d\n", shmid);
    // Asignar la estructura a la memoria compartida. datos tiene la dirección de la memoria compartida
    // shmaddr = NULL -> El SO decide la dirección donde estará el segmento de mem compartida
    datos = (struct datosCompartida *)shmat(shmid, NULL, 0);
    if (datos == (void *)-1) //(void *) -1 es un puntero a la dirección de memoria 0xFFFFFFFF
    {
        perror("Error asignando la memoria compartida");
        exit(1);
    }
    printf("Dirección mem compartida: %p\n", datos);

    // Direccion del archivo que contiene los caracteres a cargar
    //"r+": read/write y el archivo debe existir
    // datos->dataTxt = fopen("Data/charData.txt", "r+");
    // if (datos->dataTxt == NULL) {
    //     perror("Error al abrir el txt con los caracteres");
    //     exit(1);
    // }
    // printf("Puntero del txt con los datos: %p\n", datos->dataTxt);

    // // Direccion del archivo que contiene informacion de las operaciones
    // //"a+": Si el archivo no existe, lo construye
    // datos->logFile = fopen("Data/log.txt", "a");
    // if (datos->logFile == NULL) {
    //     perror("Error al abrir el log .txt");
    //     exit(1);
    // }
    // printf("Puntero del log txt: %p\n", datos->logFile);

    // Asignar valores a la estructura
    datos->clave = clave;
    datos->numeroEspacio = numeroEspacio;
    datos->indiceEmisor = 0;
    datos->indiceReceptor = 0;
    // malloc retorna un puntero al primer byte del bloque de mem
    datos->buffer = (char *)malloc((numeroEspacio) * sizeof(char));
    printf("Se asignaron %ld bytes para el buffer de caracteres.\n", (numeroEspacio) * sizeof(char));
    strncpy(datos->buffer, "sipi", numeroEspacio - 1);
    datos->buffer[numeroEspacio-1] = '\0';
    printf("buffer: %s\n", datos->buffer);
    printf("Dirección del buffer: %p \n", datos->buffer);
    printf("len buffer: %ld \n", strlen(datos->buffer));

    printDatos(datos);
    //free(datos->buffer);
    shmdt(datos);
    // sleep(25);

    // if (shmdt(datos) == -1) {  // desasignar del segmento compartido
    //     perror("Error eliminando asignacion del seg compartido");
    //     return 1;
    // }
    // fclose(dataTxt);
    return 0;
}

void printDatos(struct datosCompartida *d)
{
    printf("Clave: %d\n", d->clave);
    printf("Espacios del buffer: %d\n", d->numeroEspacio);
    printf("indiceEmisor: %d\n", d->indiceEmisor);
    printf("indiceReceptor: %d\n", d->indiceReceptor);
    // printf("contEmisoresVivos: %d\n", d->contEmisoresVivos);
    // printf("contReceptoresVivos: %d\n", d->contReceptoresVivos);
    printf("buffer: %s\n", d->buffer);
    return;
}