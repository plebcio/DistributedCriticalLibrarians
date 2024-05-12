#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

/* 
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
proc_state stan = proc_state::REST;

/* zamek wokół zmiennej współdzielonej między wątkami. 
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t lamport_clock_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t glob_data_mut = PTHREAD_MUTEX_INITIALIZER;
glob_data globals



struct tagNames_t{
    const char *name;
    mess_t tag;
} tagNames[] = { { "REQ_MPC", mess_t::REQ_MPC }, { "ACK_MPC", mess_t::ACK_MPC }, 
                { "REL_MPC", mess_t::REL_MPC }, {"REQ_SERVICE", mess_t::REQ_SERVICE }, {"ACK_SERVICE", mess_t::ACK_SERVICE } };

const char *const tag2string( mess_t tag )
{
    for (int i=0; i <sizeof(tagNames)/sizeof(struct tagNames_t);i++) {
	if ( tagNames[i].tag == to_und(tag) )  return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, data);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, mess_t tag)
{
    int freepkt=0;
    if (pkt==0) { pkt = malloc(sizeof(packet_t)); freepkt=1;}
    pkt->src = rank;
    pthread_mutex_lock(&lamport_clock_mutex);
    lamport_clock++;
    pkt->ts=lamport_clock;
    pthread_mutex_unlock(&lamport_clock_mutex);
    MPI_Send( pkt, 1, MPI_PAKIET_T, destination, to_und(tag), MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
    if (freepkt) free(pkt);
}

// clock must be incremented outside and this function MUST be used under mutex !
void broadcastPacket(packet_t *pkt, mess_t tag, int ts)
{
    int freepkt = 0;
    if (pkt==0) { 
        pkt = malloc(sizeof(packet_t)); 
        freepkt = 1;
    }
    pkt->src = rank;
    pkt->ts = ts;

    for (int i = 0; i < size; i++){
        if (i == rank) {
            continue;
        }
        
        MPI_Send( pkt, 1, MPI_PAKIET_T, i, to_und(tag), MPI_COMM_WORLD);
        debug("Wysyłam %s do %d\n", tag2string(tag), i);
    }
    
    if (freepkt) free(pkt);
}


void changeState( proc_state newState )
{
    pthread_mutex_lock( &stateMut );
    if (stan==InFinish) { 
	pthread_mutex_unlock( &stateMut );
        return;
    }
    stan = newState;
    pthread_mutex_unlock( &stateMut );
}

