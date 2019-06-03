#include "main.h"

#define procesy 16
#define stroje_min 5
#define stroje_max 10
#define lodzie_min 5
#define lodzie_max 10
#define poj_lodzi_min 3
#define poj_lodzi_max 10
MPI_Datatype MPI_PAKIET_T, MPI_PAKIET_INIT_T;
pthread_t threadCom, threadM;

int lamport,lamport_do_kucykow,lamport_do_lodzi;
int gorsze_kucyki=0, gorsze_lodzie=0;

int STAN_PROCESU=0;
pthread_mutex_t lock;

/* zamek do synchronizacji zmiennych współdzielonych */
pthread_mutex_t kucyki_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lodzie_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lamport_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t packetMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t packet_mut = PTHREAD_MUTEX_INITIALIZER;
sem_t all_sem;
// struct Turysta{
//     int info_s; //0-nie che stroju, 1-chce stroj, 2-oddał stroj
//     int info_l; //0-nie che, 1-chce, 2-oddał
// }

/* Ile każdy proces ma na początku*/
int liczba_kucykow=0;
int liczba_kucykow_p=0;
//WERSJA NA 3
int liczba_lodzi=0;
int liczba_lodzi_p=0;
int nie_kucyk = 0, nie_lodz = 0;

/* info o turystach */
//Turysta tab[procesy];

/* end == TRUE oznacza wyjście z main_loop */
volatile char end = FALSE;
volatile char init = TRUE;
volatile char end_travel = FALSE;
void mainLoop(void);

void initHandler(packet_init_t *pakiet);
void takePony(packet_t *pakiet);
void takeBoat(packet_t *pakiet);
void imBack(packet_t *pakiet);
void responsePony(packet_t *pakiet);
void responseBoat(packet_t *pakiet);

typedef void (*f_w1)(packet_t *);
typedef void (*f_w2)(packet_init_t *);

f_w1 handlers1[MAX_HANDLERS] = { 
            [TAKE_PONY] = takePony,
            [TAKE_BOAT] = takeBoat,
            [IM_BACK] = imBack,
            [RESPONSE_PONY] = responsePony,
            [RESPONSE_BOAT] = responseBoat};
f_w2 handlers2[2] = { 
            [INIT] = initHandler};

extern void inicjuj(int *argc, char ***argv);
extern void finalizuj(void);

int main(int argc, char **argv)
{
    pthread_mutex_init(&lock, NULL);
    inicjuj(&argc,&argv);

    mainLoop();

    finalizuj();
    return 0;
}

void mainLoop(void)
{
    packet_t pakiet;
    lamport=0;pakiet.ts=0;
    while(init){

    }
    while (!end)
    {
        sendPacketAll(&pakiet, TAKE_PONY, 2);
        while(size - liczba_kucykow_p > gorsze_kucyki + nie_kucyk || liczba_kucykow < 1)
        {
        }
         pthread_mutex_lock( &packetMut );
            STAN_PROCESU=3;
        pthread_mutex_unlock( &packetMut );
        println("Kucyki %i, %i, %i", size - liczba_kucykow_p, gorsze_kucyki, liczba_kucykow);
        println("Przyznano ci kucyka :) \n");
        liczba_kucykow = liczba_kucykow - gorsze_kucyki;
        gorsze_kucyki = 0;
        nie_kucyk = 0;

        sendPacketAll(&pakiet, TAKE_BOAT, 4);
        while(size - liczba_lodzi_p > gorsze_lodzie + nie_lodz || liczba_lodzi < 1)
        {
        }
        pthread_mutex_lock( &packetMut );
            STAN_PROCESU=5;
        pthread_mutex_unlock( &packetMut );
        println("Lodzie %i, %i, %i", size - liczba_lodzi_p, liczba_lodzi, gorsze_lodzie);
        println("Przyznano ci lodz :) \n");
        liczba_lodzi = liczba_lodzi - gorsze_lodzie;
        gorsze_lodzie = 0;
        nie_lodz = 0;

        int czas_rejsu = rand() %5 + 1;
        sleep(czas_rejsu);
        println("Zwolnienie1 \n");                 
        sendPacketAll(&pakiet, IM_BACK, 6);
    }
    
}

/* tylko u ROOTa */
void *monitorFunc(void *ptr)
{
    packet_init_t data;
	srand( time( NULL ) );
    liczba_kucykow = 2;// rand() % (stroje_max - stroje_min) + stroje_min;
    liczba_kucykow_p = liczba_kucykow;
    liczba_lodzi = 1;
    // for(int i = 0; i < rand() % (lodzie_max - lodzie_min) + lodzie_min; i++){
    //     Lodz lodz;
    //     lodz.pojemnosc = rand() % (poj_lodzi_max - poj_lodzi_min) + poj_lodzi_min;
    //     data.ile_lodzi= lodz.pojemnosc;
    //     //lodz.lista_id_turystow.clear();
    //     //kolejka_lodzi.push(lodz);
    //    // lodz.ile_lodzi = rand() % (poj_lodzi_max - poj_lodzi_min) + poj_lodzi_min;
    // }
    
    data.kucyki = liczba_kucykow;
    data.ile_lodzi = liczba_lodzi;
    data.ts = lamport;
        sendPacketInit(&data, 1);
    init = FALSE;
    return 0;
}

void *comFunc(void *ptr)
{
    MPI_Status status;
    if(rank != 0){
        packet_init_t pak;
        MPI_Recv( &pak, 1, MPI_PAKIET_INIT_T, MPI_ANY_SOURCE, INIT, MPI_COMM_WORLD, &status);
        handlers2[(int)status.MPI_TAG](&pak);
        pthread_mutex_lock(&packetMut);
        if(lamport<pak.ts) lamport = pak.ts;
            lamport += 1;
        pthread_mutex_unlock(&packetMut);
    }
    while ( !end ) {
        packet_t pakiet;
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pakiet.src = status.MPI_SOURCE;

        pthread_mutex_lock(&packetMut);
            if(lamport<pakiet.ts) lamport = pakiet.ts;
            lamport += 1;
        pthread_mutex_unlock(&packetMut);
        handlers1[(int)status.MPI_TAG](&pakiet);
    }
    println(" Koniec! ");
    return 0;
}

void initHandler( packet_init_t *pakiet)
{
    pthread_mutex_lock(&packetMut);
        liczba_kucykow=pakiet->kucyki;
        liczba_kucykow_p = liczba_kucykow;
        println("Stan kucyki: %d\n", liczba_kucykow);
    pthread_mutex_unlock(&packetMut);
    pthread_mutex_lock(&packetMut);
        liczba_lodzi=pakiet->ile_lodzi;
        liczba_lodzi_p=liczba_lodzi;
        println("Stan lodzie: %d\n", liczba_lodzi);
    pthread_mutex_unlock(&packetMut);
    init = FALSE;
}

void takePony( packet_t *pakiet)
{
    
    pthread_mutex_lock( &packetMut );
    int temp = STAN_PROCESU;
    pthread_mutex_unlock( &packetMut );
    if(temp==2)
    {
        if(lamport_do_kucykow == pakiet->ts)
        {
            if (pakiet->src < rank){
                liczba_kucykow--;
            }
            else{
                gorsze_kucyki++;
            }
        }
        else if(lamport_do_kucykow > pakiet->ts)
        {
            liczba_kucykow--;
        }
        else{
            gorsze_kucyki++;   
        }
    }
    else
    {
        liczba_kucykow--;
        sendPacket(pakiet, pakiet->src, RESPONSE_PONY);
    }
}

void responsePony( packet_t *pakiet){
    if(STAN_PROCESU == 2)
        nie_kucyk++; 
}

void responseBoat( packet_t *pakiet){
    if(STAN_PROCESU == 4)
        nie_lodz++;
}

void takeBoat(packet_t *pakiet)
{
    pthread_mutex_lock( &packetMut );
    int temp = STAN_PROCESU;
    pthread_mutex_unlock( &packetMut );
    if(temp==4)
    {
        if(lamport_do_lodzi == pakiet->ts)
        {
            if (pakiet->src < rank)
                liczba_lodzi--;
            else
                gorsze_lodzie++;
        }
        else if(lamport_do_kucykow > pakiet->ts)
        {
            liczba_lodzi--;
        }
        else{
            gorsze_lodzie++;   
        }
    }
    else
    {
        liczba_lodzi--;
        sendPacket(pakiet, pakiet->src, RESPONSE_BOAT);
    }
}

void imBack(packet_t *pakiet)
{
    liczba_kucykow++;
    liczba_lodzi++;
    end_travel = TRUE;
}
