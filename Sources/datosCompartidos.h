#ifndef datosCompartidos
#define datosCompartidos

//Struct que vamos a emplear para guardar en la memoria compartida
// tamano int->4 bytes, char->1 byte, buffer->256*4bytes

// with int buffer      :  4 + 16 + 8 + 8 + 1 + 100  = 20 + 16 + 101 = 137 + padding bytes from compiler 
// with char buffer   :  4 + 16 + 8 + 8 + 1 + 25   = 20 + 16 + 26  = 62 + padding bytes from compiler 
struct datosCompartida {
    int numeroEspacio;
    int indiceEmisor, indiceReceptor, indiceTxtEmisor, indiceTxtReceptor;
    int contEmisoresVivos, contReceptoresVivos;
    int contEmisoresTotal, contReceptoresTotal;
    char clave;
    char* buffer;
    FILE* dataTxt;
    FILE* logFile;
};

#endif /* datosCompartidos */