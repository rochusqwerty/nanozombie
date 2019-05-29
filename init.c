#include "main.h"

int rank;
int size;

pthread_t threadDelay;
//GQueue *delayStack;

//queue < Lodz > kolejka_lodzi;

void check_thread_support(int provided)
{
    printf("THREAD SUPPORT: %d\n", provided);
    switch (provided) {
        case MPI_THREAD_SINGLE: 
            printf("Brak wsparcia dla wątków, kończę\n");
            /* Nie ma co, trzeba wychodzić */
	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
	    MPI_Finalize();
	    exit(-1);
	    break;
        case MPI_THREAD_FUNNELED: 
            printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
	    break;
        case MPI_THREAD_SERIALIZED: 
            /* Potrzebne zamki wokół wywołań biblioteki MPI */
            printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
	    break;
        case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n");
	    break;
        default: printf("Nikt nic nie wie\n");
    }
}

/* Nie ruszać, do użytku wewnętrznego przez wątek komunikacyjny */
typedef struct {
    packet_t *newP;
    int type;
    int dst;
    } stackEl_t;


void inicjuj(int *argc, char ***argv)
{
    int provided;
    //delayStack = g_queue_new();
    MPI_Init_thread(argc, argv,MPI_THREAD_MULTIPLE, &provided);
    check_thread_support(provided);


    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    const int nitems=FIELDNO; // Struktura ma FIELDNO elementów - przy dodaniu pola zwiększ FIELDNO w main.h !
    int       blocklengths[FIELDNO] = {1,1,1,1,1}; /* tu zwiększyć na [4] = {1,1,1,1} gdy dodamy nowe pole */
    MPI_Datatype typy[FIELDNO] = {MPI_INT, MPI_INT,MPI_INT,MPI_INT,MPI_INT}; /* tu dodać typ nowego pola (np MPI_BYTE, MPI_INT) */
    MPI_Aint     offsets[FIELDNO];

    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, kod);
    offsets[2] = offsetof(packet_t, dst);
    offsets[3] = offsetof(packet_t, src);
    offsets[4] = offsetof(packet_t, waga_turysty);
    /* tutaj dodać offset nowego pola (offsets[2] = ... */

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);

    const int nitems_i=3; // Struktura ma FIELDNO elementów - przy dodaniu pola zwiększ FIELDNO w main.h !
    int       blocklengths_i[3] = {1,1,1}; /* tu zwiększyć na [4] = {1,1,1,1} gdy dodamy nowe pole */
    MPI_Datatype typy_i[3] = {MPI_INT, MPI_INT,MPI_INT}; /* tu dodać typ nowego pola (np MPI_BYTE, MPI_INT) */
    MPI_Aint     offsets_i[3];

    offsets_i[0] = offsetof(packet_init_t, ts);
    offsets_i[1] = offsetof(packet_init_t, kucyki);
    offsets_i[2] = offsetof(packet_init_t, ile_lodzi);

    MPI_Type_create_struct(nitems_i, blocklengths_i, offsets_i, typy_i, &MPI_PAKIET_INIT_T);
    MPI_Type_commit(&MPI_PAKIET_INIT_T);


    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank);

    pthread_create( &threadCom, NULL, comFunc, 0);
    //pthread_create( &threadDelay, NULL, delayFunc, 0);
    if (rank==ROOT_PROC) {
	pthread_create( &threadM, NULL, monitorFunc, 0);
    
    } 
}

void finalizuj(void)
{
    // pthread_mutex_destroy( &konto_mut);
    pthread_join(threadCom,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
    //g_queue_free(delayStack);
}

void sendPacket(packet_t *data, int dst, int type)
{
    pthread_mutex_lock( &packetMut );
	    lamport += 1;
	pthread_mutex_unlock( &packetMut );
    data->ts = lamport;
    MPI_Send( data, 1, MPI_PAKIET_T, dst, type, MPI_COMM_WORLD);
}

void sendPacketInit(packet_init_t *data, int type)
{
    pthread_mutex_lock( &packetMut );
	    lamport += 1;
	pthread_mutex_unlock( &packetMut );
    data->ts = lamport;
    //TO DO for do wszystkich
    int i;
    for(i=0; i<size; i++)
        MPI_Send( data, 1, MPI_PAKIET_INIT_T, i, type, MPI_COMM_WORLD);
}
