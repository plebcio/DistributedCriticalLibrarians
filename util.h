#ifndef UTILH
#define UTILH
#include "main.h"
#include <type_traits>
#include <vector>
#include <set>

/* typ pakietu */
struct packet_t {
    int ts;       /* timestamp (zegar lamporta */
    int src;  

    int mpc_id;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
    int mpi_state;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
    int is_Waiting;
};
/* packet_t ma cztery!! pola, więc NITEMS=5. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 5


// message types that are inplicictly int 
enum mess_t {
    REQ_MPC = 0,
    ACK_MPC,
    REL_MPC,
    REQ_SERVICE,
    ACK_SERVICE
};



extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);


void broadcastPacket(packet_t *pkt, int tag, int ts);



extern pthread_mutex_t lamport_clock_mutex;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t glob_data_mut;


/* zmiana stanu, obwarowana muteksem */
void changeState( proc_state newState );


#endif
