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
#include "datosCompartidos.h" // Estructura
#include "Constantes.h"

void setupEmisor(char *Modo, char *ID, int clave);

int main(int argc, char *argv[])
{
    // Valores compartidos
    char *Modo;
    char *ID;
    int clave;

    // Obtener los argumentos y convertir el número a entero
    Modo = argv[1];
    ID = argv[2];
    clave = atoi(argv[3]);

    char ch;
    printf("Presione <Enter> para ejecutar el emisor...");
    ch = getchar();
    if (ch == 13 || ch == 10)
    { // 13 es el valor ASCII de la tecla <Enter>
        setupEmisor(Modo, ID, clave);
    }
    else
    {
        printf("Presione la tecla <Enter>.\n");
    }
    return 0;
}

void setupEmisor(char *Modo, char *ID, int clave)
{
    printf("Modo: %s\n", Modo);
    printf("ID: %s\n", ID);
    printf("Clave: %d\n", clave);
    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;
    sem_llenos = sem_open("/sem_llenos", 0);
    sem_vacios = sem_open("/sem_vacios", 0);

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;

    // Crear una clave única para la memoria compartida
    key_t key = ftok(KEY_PATH, *ID);
    printf("key: %-20d\n", key);

    size_t tamano = sizeof(struct datosCompartida);
    printf("tamaño mem compartida: %zu bytes\n", tamano);
    // Copiamos la memoria compartida
    int shmid = shmget(key, tamano, 0666);
    if (shmid == -1)
    {
        perror("Error al obtener el ID de la mem compartida");
        exit(1);
    }
    printf("ID mem compartida: %d\n", shmid);
    // Asignar la estructura a la memoria compartida
    datos = (struct datosCompartida *)shmat(shmid, NULL, 0);
    if (datos == (void *)-1)
    {
        perror("Error al asignar la memoria compartida al proceso");
        exit(1);
    }
    printf("Dirección mem compartida: %p\n", datos);

    printf("Dirección del buffer: %p \n", datos->buffer);

    printf("Dirección de dataTxt: %p \n", datos->dataTxt);

    //sem_wait(sem_vacios);
    // Leer del txt y Escribir caracter en el buffer test
    datos->buffer = (char*)malloc((datos->numeroEspacio) * sizeof(char));
    printf("tamaño buffer: %zu bytes\n", (datos->numeroEspacio) * sizeof(char));
    
    char str[6];
    strncpy(str, (char*)datos->buffer, sizeof(str) - 1);
    printf("size of: %ld\n", sizeof(str));
    int i;
    for (i = 0; str[i] != '\0'; i++);
    if (i == sizeof(str) - 1) {
        printf("str1 is null-terminated\n");
    } else {
        printf("str1 is not null-terminated\n");
    }
    printf("char 1: %c\n", str[0]);
    printf("char 2: %c\n", str[1]);
    // copy to buffer
    strncpy(datos->buffer, "hello", (datos->numeroEspacio)-1);
    // read from buffer
    char str2[6];
    memcpy(str2, datos->buffer, 5*sizeof(char));
    printf("char after: %s\n", str2);

    free(datos->buffer);
    exit(1);

    sem_post(sem_llenos);
    // strcpy(datos->buffer, "abcd");
    // printf("Cantidad a leer: %ld \n", strlen(datos->buffer));
    exit(1);
    // size_t num_read = fread(datos->buffer, sizeof(char), sizeof(datos->buffer), datos->dataTxt);
    // datos->buffer[datos->indiceEmisor] = 'e';
    // printf("Buffer: %s \n", datos->buffer);

    // Memoria circular
    if (datos->numeroEspacio == datos->indiceEmisor)
    {
        datos->indiceEmisor = 0;
    }

    datos->contEmisoresVivos++;
    sem_post(sem_llenos);
    // Mutex

    /////////////////// Zona critica ////////////////////

    datos->buffer[0] = 1;
    datos->indiceEmisor++;
    datos->contEmisoresVivos--;
    datos->contEmisoresTotal++;

    // printf("%-20d",datos->contEmisoresTotal);
    printf("%-20d", datos->buffer[1]);
    printf("\n");

    time_t tiempo_actual = time(NULL);                   // Obtenemos el tiempo actual en segundos
    struct tm *tiempo_local = localtime(&tiempo_actual); // Convertimos el tiempo en una estructura tm
    printf("Fecha actual: %d/%d/%d\n", tiempo_local->tm_year + 1900, tiempo_local->tm_mon + 1, tiempo_local->tm_mday);
    printf("Hora actual: %d:%02d:%02d\n", tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec);

    sem_wait(sem_vacios);
}
