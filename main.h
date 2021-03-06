#ifndef MAINH
#define MAINH

/* boolean */
#define TRUE 1
#define FALSE 0

#define ROOT_PROC 0

#define INIT 1
#define TAKE_PONY 2
#define TAKE_BOAT 3
#define IM_BACK 4
#define RESPONSE_PONY 5
#define RESPONSE_BOAT 6
/* MAX_HANDLERS musi się równać wartości ostatniego typu pakietu + 1 */
#define MAX_HANDLERS 7

#include <mpi.h>
#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
//#include <queue>

/* FIELDNO: liczba pól w strukturze packet_t */
#define FIELDNO 5 //4
typedef struct {
    int ts; /* zegar lamporta */
    int kod; //0 - zadanie  1 - odpowiedz   chce kucyk, 1 - nie chce, 2 - zwalnia kucyka, 3 - chce lodz, 4 - nie chce lodzi, 5 - zwalnia lodz


    int dst; /* pole ustawiane w sendPacket */
    int src; /* pole ustawiane w wątku komunikacyjnym na rank nadawcy */
    /* przy dodaniu nowych pól zwiększy FIELDNO i zmodyfikuj 
       plik init.c od linijki 76
    */
    int waga_turysty;
} packet_t;

typedef struct{
    int pojemnosc;
    //std::queue < int > lista_id_turystow;
}Lodz;

typedef struct {
    int ts;
    int kucyki;
    int ile_lodzi;
    //std::queue < Lodz > kolejka; 
} packet_init_t;


//extern std::queue < Lodz > kolejka_lodzi;
extern int rank,size;
extern volatile char end;
extern MPI_Datatype MPI_PAKIET_T, MPI_PAKIET_INIT_T;
extern pthread_t threadCom, threadM, threadDelay;

/* do użytku wewnętrznego (implementacja opóźnień komunikacyjnych) */
//extern GQueue *delayStack;
/* synchro do zmiennej konto */
extern pthread_mutex_t kucyki_mut;
extern pthread_mutex_t lodzie_mut;
extern pthread_mutex_t packetMut;

/* argument musi być, bo wymaga tego pthreads. Wątek monitora, który po jakimś czasie ma wykryć stan */
extern void *monitorFunc(void *);
/* argument musi być, bo wymaga tego pthreads. Wątek komunikacyjny */
extern void *comFunc(void *);

extern void sendPacket(packet_t *, int, int);
extern void sendPacketAll(packet_t *, int, int);
extern void sendPacketInit(packet_init_t *, int);

#define PROB_OF_SENDING 35
#define PROB_OF_PASSIVE 5
#define PROB_OF_SENDING_DECREASE 1
#define PROB_SENDING_LOWER_LIMIT 1
#define PROB_OF_PASSIVE_INCREASE 1

/* makra do wypisywania na ekranie */
#define P_WHITE printf("%c[%d;%dm",27,1,37);
#define P_BLACK printf("%c[%d;%dm",27,1,30);
#define P_RED printf("%c[%d;%dm",27,1,31);
#define P_GREEN printf("%c[%d;%dm",27,1,33);
#define P_BLUE printf("%c[%d;%dm",27,1,34);
#define P_MAGENTA printf("%c[%d;%dm",27,1,35);
#define P_CYAN printf("%c[%d;%d;%dm",27,1,36);
#define P_SET(X) printf("%c[%d;%dm",27,1,31+(6+X)%7);
#define P_CLR printf("%c[%d;%dm",27,0,37);

/* Tutaj dodaj odwołanie do zegara lamporta */
extern int lamport, lamport_do_kucykow, lamport_do_lodzi, STAN_PROCESU;
#define println(FORMAT, ...); printf("%c[%d;%dm [lam:%d %d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, lamport, rank, ##__VA_ARGS__, 27,0,37);

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.
*/
#ifdef DEBUG
#define debug(...) printf("%c[%d;%dm [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, ##__VA_ARGS__, 27,0,37);

#else
#define debug(...) ;
#endif
#endif
