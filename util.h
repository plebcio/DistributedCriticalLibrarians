#ifndef UTILH
#define UTILH
#include "main.h"
#include <type_traits>
#include <vector>
#include <set>

/* typ pakietu */
struct packet_t {
    int ts;
    int src;  

    int mpc_id;
    int mpi_state;
    int is_Waiting;
};
#define NITEMS 5


enum mess_t {
    REQ_MPC = 0,
    ACK_MPC,
    REL_MPC,
    REQ_SERVICE,
    ACK_SERVICE
};



extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();


void sendPacket(packet_t *pkt, int destination, int tag);
void broadcastPacket(packet_t *pkt, int tag, int ts);
void sendPacketNoIncOnTs(packet_t *pkt, int destination, int tag);



extern pthread_mutex_t lamport_clock_mutex;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t glob_data_mut;


/* zmiana stanu, obwarowana muteksem */
void changeState( proc_state newState );


#endif
