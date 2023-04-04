#ifndef datosCompartidos
#define datosCompartidos

struct datosCompartida {
    int numeroEspacio;
    int indiceEmisor, indiceReceptor, indiceTxtEmisor, indiceTxtReceptor;
    int contEmisoresVivos, contReceptoresVivos;
    int contEmisoresTotal, contReceptoresTotal;
    char clave;
    int *buffer;
};

#endif /* datosCompartidos */