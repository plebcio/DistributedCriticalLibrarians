#include "main.h"
#include "watek_komunikacyjny.h"
#include <algorithm>
#include "util.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( true ) {
	    debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&lamport_clock_mutex);
        lamport_clock = std::max(lamport_clock, pakiet.ts) + 1;
        pthread_mutex_unlock(&lamport_clock_mutex);

        switch ( stan ) {
        case proc_state::REST: {
            switch ( status.MPI_TAG ) {
                case mess_t::REQ_MPC: {
                    int& sender = pakiet.src;
                    globals.lock();
                    globals.MPCWaitQueueArray[pakiet.mpc_id].emplace( pakiet.ts, pakiet.proc_id  );
                    globals.unlock();

                    pakiet.is_Waiting = 0;

                    sendPacket(&pakiet, sender, mess_t::ACK_MPC);
                } break;
            case mess_t::ACK_MPC: {
                println("GOT ACK_MPC in state REST WENT WRONGG!!!!");
            } break;
            case mess_t::REL_MPC: {
                globals.lock();
                globals.MPCStateArray[pakiet.mpc_id] = pakiet.mpi_state;
                auto it = globals.MPCWaitQueueArray[pakiet.mpc_id].find_if(
                    globals.MPCWaitQueueArray[pakiet.mpc_id].begin(),
                    globals.MPCWaitQueueArray[pakiet.mpc_id].end(),
                    [pakiet.src](packet_t const& p){
                        return p.proc_id == pakiet.src; 
                    });
                if (it == globals.MPCStateArray[pakiet.mpc_id].end()){
                    println("No request in the MPC queue --  SOMETHING WENT WRONGG!!!!");
                    exit(0x45);
                }
                globals.MPCStateArray[pakiet.mpc_id].erase(it);
                globals.unlock();
            } break;
            case mess_t::REQ_SERVICE: {
                sendPacket(&pakiet, pakiet.src, mess_t::ACK_SERVICE);
            } break;
            case mess_t::ACK_SERVICE: {
                globals.lock();
                globals.ServiceAckNum[pakiet.src] -= 1;
                globals.unlock();
            } break;
            }
        } break;
        

        }


        switch ( status.MPI_TAG ) {
	    case mess_t:: 
                debug("Ktoś coś prosi. A niech ma!")
		    sendPacket( 0, status.MPI_SOURCE, ACK );
	        break;
	    case ACK: 
                debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
	        ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
	    break;
	    default:
	    break;
        }
    }
}
