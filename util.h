#ifndef UTILH
#define UTILH
#include "main.h"
#include <type_traits>
#include <vector>
#include <set>

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  

    int mpc_id;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
    int mpi_state;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
    int is_Waiting;
} packet_t;
/* packet_t ma cztery!! pola, więc NITEMS=5. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 5



// message types that are inplicictly int 
enum class mess_t {
    REQ_MPC = 0,
    ACK_MPC,
    REL_MPC,
    REQ_SERVICE,
    ACK_SERVICE
};

template <typename E>
constexpr auto to_und(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

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

extern pthread_mutex_t glob_data_mut;

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

    void lock(){
        pthread_mutex_lock( &glob_data_mut );
    }

    void unlock(){
    	pthread_mutex_unlock( &glob_data_mut );
    }

    glob_data(int n_proc){
        MPCWaitQueueArray = std::vector<std::set<request>>(n_proc, std::set<request>());
        MPCStateArray = std::vector<int> (NUM_MPC, BASE_MPC_STATE);
        ServiceReqNum = std::vector<int> (n_proc, 0);
    }
};

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, mess_t tag);


void broadcastPacket(packet_t *pkt, mess_t tag, int ts);


enum class proc_state { REST, WAIT_MPC, INSECTION_MPC, WAIT_SERVICE, INSECTION_SERVICE };

extern proc_state stan;
extern pthread_mutex_t lamport_clock_mutex;
extern pthread_mutex_t stateMut;

extern glob_data globals;

/* zmiana stanu, obwarowana muteksem */
void changeState( proc_state newState );


#endif
