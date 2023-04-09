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

void setupEmisor(char *Modo, char *ID, int clave);
void printDatos(struct datosCompartida *d);

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
    /* Inicializar semáforos */
    sem_t *sem_llenos, *sem_vacios;
    sem_llenos = sem_open("/sem_llenos", 0);
    sem_vacios = sem_open("/sem_vacios", 0);

    // Inicializamos esta memoria compartida
    struct datosCompartida *datos;

    // Crear una clave única para la memoria compartida
    key_t key = ftok(KEY_PATH, *ID);
    printf("key: %d\n", key);

    size_t strucTamano = sizeof(struct datosCompartida);
    // Copiamos la memoria compartida
    int shmid = shmget(key, strucTamano, 0666);
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

    //---Debug
    struct shmid_ds segment_info;
    shmctl(shmid, IPC_STAT, &segment_info);
    printf("Current size of shared memory segment: %ld\n", segment_info.shm_segsz);
    printf("No. of current attaches: %ld\n", segment_info.shm_nattch);
    printf("Owner: %d. My PID: %d \n", segment_info.shm_cpid, getpid());
    //--------

    // Direccion del archivo que contiene los caracteres a cargar
    // "r+": read/write y el archivo debe existir
    FILE* dataTxt;
    dataTxt = fopen("Data/charData.txt", "r+");
    if (dataTxt == NULL) {
        perror("Error al abrir el txt con los caracteres");
        exit(1);
    }
    printf("Puntero del txt con los datos: %p\n", dataTxt);

    // Direccion del archivo que contiene informacion de las operaciones
    //"a+": Si el archivo no existe, lo construye
    FILE* logFile;
    logFile = fopen("Data/log.txt", "a");
    if (logFile == NULL) {
        perror("Error al abrir el log .txt");
        exit(1);
    }
    printf("Puntero del log txt: %p\n", logFile);


    //////// REGION CRITICA ////////
    sem_wait(sem_vacios);
    //mutex here

    // leer un unico caracter del archivo txt de los datos
    fseek(dataTxt, datos->indiceTxtEmisor, SEEK_SET);   //mover el cursor del file a donde lo dejo el emisor anterior
    char charLeido;
    size_t num_read = fread(&charLeido, sizeof(char), 1, dataTxt);  // 1 significa que un solo caracter se va a leer
    if(num_read =! 1){
        perror("Error al leer un caracter del txt");
        fclose(dataTxt);
        exit(1);
    }
    printf("Char read from file: %c \n", charLeido);
    datos->indiceTxtEmisor++;

    // copiar el caracter leido al sitio respectivo del buffer
    datos->buffer[datos->indiceEmisor] = charLeido;
    printf("Buffer : %s \n", datos->buffer);
    printf("Indice emisor : %d \n", datos->indiceEmisor);
    
    
    // escribir la info en el log file, se escribe una linea al final del archivo
    char infoFormato[] = "%d-%c    | %c           |  %d       |  %s \n";
    fprintf(logFile, infoFormato, getpid(),'E', charLeido, datos->indiceEmisor, "dd:mm:ss");
    
    //Buffer con R/W circular
    datos->indiceEmisor++;
    if (datos->indiceEmisor >= datos->numeroEspacio)
    {
        datos->indiceEmisor = 0;
    }

    //printDatos(datos);
    //unlock mutex
    sem_post(sem_llenos);

    if (shmdt(datos) == -1) {  // desasignar del segmento compartido
        perror("Error eliminando asignacion del seg compartido");
        exit(1);
    }
    fclose(logFile);
    fclose(dataTxt);

    exit(1);

    //strncpy(datos->buffer, "hello", (datos->numeroEspacio)-1);
    //memcpy(str2, datos->buffer, 5*sizeof(char));

    /////////////////////////////////


}
void printDatos(struct datosCompartida *d)
{
    printf("--- Printing shared memory... --- \n");
    printf("Clave: %d\n", d->clave);
    printf("Espacios del buffer: %d\n", d->numeroEspacio);
    printf("indiceEmisor: %d\n", d->indiceEmisor);
    printf("indiceReceptor: %d\n", d->indiceReceptor);
    printf("indiceTxtEmisor: %d\n", d->indiceTxtEmisor);
    printf("indiceTxtReceptor: %d\n", d->indiceTxtReceptor);
    printf("Buffer: %s\n", d->buffer);
    printf("Dirección del buffer: %p \n", d->buffer);
    printf("Length buffer: %ld \n", strlen(d->buffer));
    return;
}


    // time_t tiempo_actual = time(NULL);                   // Obtenemos el tiempo actual en segundos
    // struct tm *tiempo_local = localtime(&tiempo_actual); // Convertimos el tiempo en una estructura tm
    // printf("Fecha actual: %d/%d/%d\n", tiempo_local->tm_year + 1900, tiempo_local->tm_mon + 1, tiempo_local->tm_mday);
    // printf("Hora actual: %d:%02d:%02d\n", tiempo_local->tm_hour, tiempo_local->tm_min, tiempo_local->tm_sec);
