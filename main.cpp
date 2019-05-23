#include "main.h"

#define procesy 16
#define stroje_min 10
#define stroje_max 10
#define lodzie_min 10
#define lodzie_max 10
#define poj_lodzi_min 10
#define poj_lodzi_max 10
MPI_Datatype MPI_PAKIET_T;
pthread_t threadCom, threadM;

int lamport,lamport_do_kucykow,lamport_do_lodzi;
int gorsze_kucyki=0, gorsze_lodzie=0;
// int zadania2=0;
int STAN_PROCESU=0;
pthread_mutex_t lock;

bool decyzja=false;
bool decyzja2=false;

/* zamek do synchronizacji zmiennych współdzielonych */
pthread_mutex_t stroje_mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lodzie_mut = PTHREAD_MUTEX_INITIALIZER;
sem_t all_sem;
// struct Turysta{
//     int info_s; //0-nie che stroju, 1-chce stroj, 2-oddał stroj
//     int info_l; //0-nie che, 1-chce, 2-oddał
// }

/* Ile każdy proces ma na początku*/
int liczba_kucykow=0;
//WERSJA NA 3
int liczba_lodzi = kolejka_lodzi.size();

/* info o turystach */
//Turysta tab[procesy];

/* end == TRUE oznacza wyjście z main_loop */
volatile char end = FALSE;
void mainLoop(void);

/* Deklaracje zapowiadające handlerów. */
void myStateHandler(packet_t *pakiet);
void finishHandler(packet_t *pakiet);
void appMsgHandler(packet_t *pakiet);
void giveHandler(packet_t *pakiet);
void initHandler(packet_t *pakiet);
void takePony(packet_t *pakiet);
void takeBoat(packet_t *pakiet);
// void imback(packet_t *pakiet);

/* typ wskaźnik na funkcję zwracającej void i z argumentem packet_t* */
typedef void (*f_w)(packet_t *);
/* Lista handlerów dla otrzymanych pakietów
   Nowe typy wiadomości dodaj w main.h, a potem tutaj dodaj wskaźnik do 
     handlera.
   Funkcje handleróœ są na końcu pliku. Nie zapomnij dodać
     deklaracji zapowiadającej funkcji!
*/
f_w handlers[MAX_HANDLERS] = { [GIVE_YOUR_STATE]=giveHandler,
            [FINISH] = finishHandler,
            [MY_STATE_IS] = myStateHandler,
            [APP_MSG] = appMsgHandler,
            [INIT] = initHandler,
            [TAKE_PONY] = takePony,
            [TAKE_BOAT] = takeBoat,
            [IM_BACK] = imBack};

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
    int prob_of_sending=PROB_OF_SENDING;
    int dst;
    packet_t pakiet;
    lamport=0;pakiet.ts=0;
    
    /* pętla główna: sen, wysyłanie przelewów innym bankom */
    
    while (!end)
    {
        decyzja=false;
        decyzja2=false;
        int i=0;

        zadania = 0;
        ile_pozostalo = size;
        ile_pozostalo2 = size;

        STAN_PROCESU=1;
        liczba_kucykow-=gorsze;
        pthread_mutex_lock( &packetMut );
            lamport_do_kucykow = lamport;
        pthread_mutex_unlock( &packetMut );
        pakiet->kod=0;
        for(i=0; i<size; i++)
            sendPacket(&pakiet, i, TAKE_PONY);
        //sem_wait(&all_sem);
        while(size - liczba_kucykow > gorsze_kucyki)
        {
            STAN_PROCESU=2;

        }
        printf("Przyznano ci kucyka :) \n");
        liczba_kucykow--;
        while (!end)
        {
            zadania2 = 0;
            pthread_mutex_lock( &packetMut );
                lamport_do_lodzi = lamport;
            pthread_mutex_unlock( &packetMut );
            pakiet->kod=0;
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
            int randomTime = rand() %50 + 1;
            sleep((float) czas_rejsu/10);
            printf("Powrót z rejsu \n");
            liczba_kucykow++;
            liczba_lodzi++;                   
            for(i=0; i<size; i++)
                sendPacket(&pakiet, i, IM_BACK);
            STAN_PROCESU=7;

        }

    }

        // if ((percent < prob_of_sending ) && (konto >0)) {
        //         /* losujemy, komu wysłać przelew */
        //     do {
        //     dst = rand()%(size);
        //     //if (dst==size) MPI_Abort(MPI_COMM_WORLD,1); // strzeżonego :D
        //     } while (dst==rank);

        //         /* losuję, ile kasy komuś wysłać */
        //     percent = rand()%konto;
        //     pakiet.kasa = percent;
        //     pthread_mutex_lock(&konto_mut);
        //         konto-=percent;
        //     pthread_mutex_unlock(&konto_mut);
        //     sendPacket(&pakiet, dst, APP_MSG);
        //         /* z biegiem czasu coraz rzadziej wysyłamy (przyda się do wykrywania zakończenia) */
        //     if (prob_of_sending > PROB_SENDING_LOWER_LIMIT)
        //     prob_of_sending -= PROB_OF_SENDING_DECREASE;

        //     println("-> wysłałem %d do %d\n", pakiet.kasa, dst);
        //     }  
    
}

/* tylko u ROOTa */
void *monitorFunc(void *ptr)
{
    packet_init_t data;
	srand( time( NULL ) );
    liczba_kucykow = rand() % (stroje_max - stroje_min) + stroje_min;
    for(int i = 0; i < rand() % (lodzie_max - lodzie_min) + lodzie_min; i++){
        Lodz lodz = Lodz();
        lodz.pojemnosc = rand() % (poj_lodzi_max - poj_lodzi_min) + poj_lodzi_min;
        lodz.lista_id_turystow.clear();
        kolejka_lodzi.push(lodz);
    }
    data.kucyki = liczba_kucykow;
    data.kolejka = kolejka_lodzi;

    for (i=0;i<size;i++)  {
	    sendPacket(&data, i, GIVE_YOUR_STATE);
    }
    sem_wait(&all_sem);

    for (i=1;i<size;i++) {
	sendPacket(&data, i, FINISH);
    }
    sendPacket(&data, 0, FINISH);
    P_RED; printf("\n\tW systemie jest: [%d]\n\n", sum);P_CLR
    return 0;
}

/* Wątek komunikacyjny - dla każdej otrzymanej wiadomości wywołuje jej handler */
void *comFunc(void *ptr)
{

    MPI_Status status;
    packet_t pakiet;
    /* odbieranie wiadomości */
    while ( !end ) {
	println("[%d] czeka na recv\n", rank);
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pakiet.src = status.MPI_SOURCE;
        

        if (status.MPI_TAG == FINISH) end = TRUE;
        else handlers[(int)status.MPI_TAG](&pakiet);
        pthread_mutex_lock(&lock);
            if(lamport<pakiet.ts) lamport = pakiet.ts;
            lamport += 1;
        pthread_mutex_unlock(&lock);
    }
    println(" Koniec! ");
    return 0;
}

/* Handlery */
void myStateHandler(packet_t *pakiet)
{
    static int statePacketsCnt = 0;

    statePacketsCnt++;
    sum += pakiet->kasa;
    println("Suma otrzymana: %d, total: %d\n", pakiet->kasa, sum);
    //println( "%d statePackets from %d\n", statePacketsCnt, pakiet->src);
    if (statePacketsCnt == size ) {
        sem_post(&all_sem);
    }
    
}

void finishHandler( packet_t *pakiet)
{
    /* właściwie nie wykorzystywane */
    println("Otrzymałem FINISH" );
    end = TRUE; 
}

void giveHandler( packet_t *pakiet)
{
    /* monitor prosi, by mu podać stan kasy */
    /* tutaj odpowiadamy monitorowi, ile mamy kasy. Pamiętać o muteksach! */
    println("dostałem GIVE STATE\n");
   
    packet_t tmp;
    tmp.kasa = konto; 
    sendPacket(&tmp, ROOT, MY_STATE_IS);
}

void appMsgHandler( packet_t *pakiet)
{
    /* ktoś przysłał mi przelew */
    println("\tdostałem %d od %d\n", pakiet->kasa, pakiet->src);
    pthread_mutex_lock(&konto_mut);
	konto+=pakiet->kasa;
    println("Stan obecny: %d\n", konto);
    pthread_mutex_unlock(&konto_mut);
}

void initHandler( packet_init_t *pakiet)
{
    /* ktoś przysłał mi przelew */
    println("\tdostałem %d\n", pakiet->kucyki);
    pthread_mutex_lock(&kucyki_mut);
        liczba_kucykow=pakiet->kucyki;
        println("Stan kucyki: %d\n", liczba_kucykow);
    pthread_mutex_unlock(&kucyki_mut);
    pthread_mutex_lock(&lodzie_mut);
        kolejka_lodzi=pakiet->kolejka;
        println("Stan lodzie: %d\n", kolejka_lodzi.size());
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
            sendPacket(&pakiet, pakiet->src, TAKE_PONY);
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
            sendPacket(&pakiet, pakiet->src, TAKE_BOAT);
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
    printf("Przypłynęła nowa łódź \n")    
}
