#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
#define STATE_CHANGE_PROB 10

#define NUM_MPC 6
#define BASE_MPC_STATE 10
#define NUM_SERVIS 5


#define ROOT 0

/* tutaj TYLKO zapowiedzi - definicje w main.c */
extern int rank;
extern int size;
extern int ackCount;
extern int lamport_clock;
extern pthread_t threadKom;

extern proc_state stan;
extern glob_data globals;



/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta 
   
   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
					   "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape. 
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów
                                            
*/
#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [%d:%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamport_clock, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT,...) printf("%c[%d;%dm [%d:%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, rank, lamport_clock, ##__VA_ARGS__, 27,0,37);


struct request {
    int ts;
    int proc_id;

    request(int ts, int proc_id): ts(ts), proc_id(proc_id) {}

};

bool operator<(request const &a, request const &b) {
    if (a.ts < b.ts){
        return true;
    }
    if (a.ts == b.ts and a.proc_id < b.proc_id){
        return true;
    }
    return false;
}


struct glob_data {
    // kolejka procesów oczekujących na ACK_SERVICE do serwisantów, początkowo pusta.
    std::vector<request> ServiceWaitQueue;
    // liczba otrzymanych potwierdzeń ACK_SERVICE do serwisantów, początkowo 0.
    int ServiceAckNum = 0;
    // liczba wysłanych REQ_SERVICE do danych procesów 0..n-1, początkowo 0.
    std::vector<int> ServiceReqNum;
    // tablica zawierająca kolejki procesów oczekujących na ACK_MPC do MPC o danych indeksach, kolejki początkowo puste.
    std::vector<std::set<request>> MPCWaitQueueArray;
    // liczba otrzymanych potwierdzeń ACK_MPC czekając do MPC
    int MPCAckNum = 0;
    std::vector<int> MPCStateArray;
    // index wybranego MPC
    int MPCIdx = 0;
    int ReadersRand = 0;

    void lock();
    void unlock();
    glob_data(int n_proc);
};

#endif
