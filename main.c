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
sem_t all_sem;
// struct Turysta{
//     int info_s; //0-nie che stroju, 1-chce stroj, 2-oddał stroj
//     int info_l; //0-nie che, 1-chce, 2-oddał
// }

/* Ile każdy proces ma na początku*/
int liczba_kucykow=0;
//WERSJA NA 3
int liczba_lodzi;

/* info o turystach */
//Turysta tab[procesy];

/* end == TRUE oznacza wyjście z main_loop */
volatile char end = FALSE;
volatile char end_travel = FALSE;
void mainLoop(void);

/* Deklaracje zapowiadające handlerów. */
void initHandler(packet_init_t *pakiet);
void takePony(packet_t *pakiet);
void takeBoat(packet_t *pakiet);
void imBack(packet_t *pakiet);

/* typ wskaźnik na funkcję zwracającej void i z argumentem packet_t* */
typedef void (*f_w1)(packet_t *);
typedef void (*f_w2)(packet_init_t *);
/* Lista handlerów dla otrzymanych pakietów
   Nowe typy wiadomości dodaj w main.h, a potem tutaj dodaj wskaźnik do 
     handlera.
   Funkcje handleróœ są na końcu pliku. Nie zapomnij dodać
     deklaracji zapowiadającej funkcji!
*/
f_w1 handlers1[MAX_HANDLERS] = { 
            [TAKE_PONY] = takePony,
            [TAKE_BOAT] = takeBoat,
            [IM_BACK] = imBack };
f_w2 handlers2[2] = { 
            [INIT] = initHandler};

extern void inicjuj(int *argc, char ***argv);
extern void finalizuj(void);

int main(int argc, char **argv)
{
    
        
    /* Tworzenie wątków, inicjalizacja itp */
    pthread_mutex_init(&lock, NULL);
    //pthread_mutex_lock(&lock);
    inicjuj(&argc,&argv);

    mainLoop();

    finalizuj();
    return 0;
}


/* Wątek główny - przesyłający innym pieniądze */
void mainLoop(void)
{
    // int dst;
    packet_t pakiet;
    lamport=0;pakiet.ts=0;
    
    /* pętla główna: sen, wysyłanie przelewów innym bankom */
    while (!end)
    {
        int i=0;

        STAN_PROCESU=1;
        liczba_kucykow-=gorsze_kucyki;
        pthread_mutex_lock( &packetMut );
            lamport_do_kucykow = lamport;
        pthread_mutex_unlock( &packetMut );
        pakiet.kod=0;
        for(i=0; i<size; i++)
            sendPacket(&pakiet, i, TAKE_PONY);
        // sem_wait(&all_sem);
        println("Kucyki %i, %i, %i", size, liczba_kucykow, gorsze_kucyki);
        while(size - liczba_kucykow > gorsze_kucyki)
        {
            STAN_PROCESU=2;
            println("Kucyki %i, %i, %i", size, liczba_kucykow, gorsze_kucyki);
            sleep(500);
        }
        println("Przyznano ci kucyka :) \n");
        liczba_kucykow--;
        while (!end_travel)
        {
            pthread_mutex_lock( &packetMut );
                lamport_do_lodzi = lamport;
            pthread_mutex_unlock( &packetMut );
            pakiet.kod=0;
            for(i=0; i<size; i++)
                sendPacket(&pakiet, i, TAKE_BOAT);
            while(size - liczba_lodzi > gorsze_lodzie)
            {
                STAN_PROCESU=4;
                
            }
            printf("Przyznano ci lodz :) \n");
            liczba_lodzi--;
            STAN_PROCESU=5;

            //WYPŁYNIĘCIE
            STAN_PROCESU=6;
            //REJS
            int czas_rejsu = rand() %50 + 1;
            sleep((float) czas_rejsu/10);
            printf("Powrót z rejsu \n");
            liczba_kucykow++;
            liczba_lodzi++;                   
            for(i=0; i<size; i++)
                sendPacket(&pakiet, i, IM_BACK);
            STAN_PROCESU=7;

        }
        end_travel = FALSE;

    }
    
}

/* tylko u ROOTa */
void *monitorFunc(void *ptr)
{
    packet_init_t data;
	srand( time( NULL ) );
    liczba_kucykow = 1;// rand() % (stroje_max - stroje_min) + stroje_min;
    liczba_lodzi = 1;
    for(int i = 0; i < rand() % (lodzie_max - lodzie_min) + lodzie_min; i++){
        Lodz lodz;
        lodz.pojemnosc = rand() % (poj_lodzi_max - poj_lodzi_min) + poj_lodzi_min;
        data.ile_lodzi= lodz.pojemnosc;
        //lodz.lista_id_turystow.clear();
        //kolejka_lodzi.push(lodz);
       // lodz.ile_lodzi = rand() % (poj_lodzi_max - poj_lodzi_min) + poj_lodzi_min;
    }
    
    data.kucyki = liczba_kucykow;
    data.ile_lodzi = liczba_lodzi;
    data.ts = lamport;
        sendPacketInit(&data, 1);
    // data.kolejka = kolejka_lodzi;
    return 0;
}

/* Wątek komunikacyjny - dla każdej otrzymanej wiadomości wywołuje jej handler */
void *comFunc(void *ptr)
{
    MPI_Status status;
    if(rank != 0){
        packet_init_t pak;
        println("[%d] czeka na init\n", rank);
        MPI_Recv( &pak, 1, MPI_PAKIET_INIT_T, MPI_ANY_SOURCE, INIT, MPI_COMM_WORLD, &status);
        handlers2[(int)status.MPI_TAG](&pak);
        pthread_mutex_lock(&lock);
        if(lamport<pak.ts) lamport = pak.ts;
            lamport += 1;
        pthread_mutex_unlock(&lock);
    }

    packet_t pakiet;
    /* odbieranie wiadomości */
    while ( !end ) {
	println("[%d] czeka na recv\n", rank);
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pakiet.src = status.MPI_SOURCE;
        

        //if (status.MPI_TAG == FINISH) end = TRUE;
        handlers1[(int)status.MPI_TAG](&pakiet);
        pthread_mutex_lock(&lock);
            if(lamport<pakiet.ts) lamport = pakiet.ts;
            lamport += 1;
        pthread_mutex_unlock(&lock);
    }
    println(" Koniec! ");
    return 0;
}

/* Handlery */

void initHandler( packet_init_t *pakiet)
{
    /* ktoś przysłał mi przelew */
    println("\tdostałem %d\n", pakiet->kucyki);
    pthread_mutex_lock(&kucyki_mut);
        liczba_kucykow=pakiet->kucyki;
        println("Stan kucyki: %d\n", liczba_kucykow);
    pthread_mutex_unlock(&kucyki_mut);
    pthread_mutex_lock(&lodzie_mut);
        liczba_lodzi=pakiet->ile_lodzi//kolejka;
        println("Stan lodzie: %d\n", liczba_lodzi);//kolejka_lodzi.size());
    pthread_mutex_unlock(&lodzie_mut);
}


void takePony( packet_t *pakiet)
{
    //ile_pozostalo--;
    if(lamport_do_kucykow == pakiet->ts)
    {
        if (pakiet->src < rank)
            liczba_kucykow--;
        else
            gorsze_kucyki++;
    }
    else if(lamport_do_kucykow > pakiet->ts)
    {
        liczba_kucykow--;
        if(pakiet->kod == 0)
        {
            sendPacket(pakiet, pakiet->src, TAKE_PONY);
            pakiet->kod = 1;
        }
    }
    else
        gorsze_kucyki++;   
}

void takeBoat(packet_t *pakiet)
{
    //ile_pozostalo--;
    if(lamport_do_lodzi == pakiet->ts)
    {
        if (pakiet->src < rank)
            liczba_lodzi--;
        else
            gorsze_lodzie++;
    }
    else if(lamport_do_lodzi > pakiet->ts)
    {
        liczba_lodzi--;
        if(pakiet->kod == 0)
        {
            sendPacket(pakiet, pakiet->src, TAKE_BOAT);
            pakiet->kod = 1;
        }
    }
    else
        gorsze_lodzie++;  
    
}

void imBack(packet_t *pakiet)
{
    liczba_kucykow++;
    liczba_lodzi++;
    printf("Przypłynęła nowa łódź \n");   
    end_travel = TRUE;
}
