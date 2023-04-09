#ifndef datosCompartidos
#define datosCompartidos

//Struct que vamos a emplear para guardar en la memoria compartida
// tamano int->4 bytes, char->1 byte, buffer->x

struct datosCompartida {
    int numeroEspacio;
    int indiceEmisor, indiceReceptor, indiceTxtEmisor, indiceTxtReceptor;
    int contEmisoresVivos, contReceptoresVivos;
    int contEmisoresTotal, contReceptoresTotal;
    char clave;
    char buffer[0];
};

#endif /* datosCompartidos */