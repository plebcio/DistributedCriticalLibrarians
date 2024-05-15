#include <algorithm>

#include "main.h"
#include "watek_komunikacyjny.h"
#include "util.h"

// odsyła ACK_MPC z odpowiednim ts oraz aktualizuje kolejki.
void rel_req_mpc(packet_t & pakiet){
    int& sender = pakiet.src;
    globals.lock();
    globals.MPCWaitQueueArray[pakiet.mpc_id].emplace( pakiet.ts, pakiet.src  );
    globals.unlock();

    pakiet.is_Waiting = 0;

    sendPacket(&pakiet, sender, ACK_MPC);
}

// REL_MPC: aktualizuje MPCStateArray[REL_MPC.MPCIdx] := REL_MPC.MPCState, 
// usuwa proces nadawcę z kolejki MPCWaitQueueArray[REL_MPC.MPCIdx].
void rel_mpc_react(packet_t const& pakiet){
    globals.lock();
    globals.MPCStateArray[pakiet.mpc_id] = pakiet.mpi_state;

    auto it = std::find_if(
        globals.MPCWaitQueueArray[pakiet.mpc_id].begin(),
        globals.MPCWaitQueueArray[pakiet.mpc_id].end(),
        [x = pakiet.src](request const& p){
            return p.proc_id == x; 
        });

    if (it == globals.MPCWaitQueueArray[pakiet.mpc_id].end()){
        println("No request in the MPC queue --  SOMETHING WENT WRONGG!!!!");
        exit(0x45);
    }
    globals.MPCWaitQueueArray[pakiet.mpc_id].erase(it);
    globals.unlock();
}

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( true ) {
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&lamport_clock_mutex);
        lamport_clock = std::max(lamport_clock, pakiet.ts) + 1;
        pthread_mutex_unlock(&lamport_clock_mutex);

        switch ( stan ) {
        case proc_state::REST: {
            switch ( status.MPI_TAG ) {
            case REQ_MPC: {
                rel_req_mpc(pakiet);
            } break;

            case ACK_MPC: {
                println("ERROR: GOT ACK_MPC in state REST. SOMETHING WENT WRONGG!!!!");
            } break;

            case REL_MPC: {
                rel_mpc_react(pakiet);
            } break;

            case REQ_SERVICE: {
                sendPacket(&pakiet, pakiet.src, ACK_SERVICE);
            } break;
            
            case ACK_SERVICE: {
                globals.lock();
                globals.ServiceReqNum[pakiet.src] -= 1;
                globals.unlock();
            } break;
            }
        } break;

        case proc_state::WAIT_MPC: {
            switch ( status.MPI_TAG ) {
            case REQ_MPC: {
                rel_req_mpc(pakiet);
            } break;

            case ACK_MPC: {
                globals.lock();
                // TODO 
                // Str 4. Sprawdza ts – powinien być większy niż ts REQ dokonanego przez proces odbierający (sprawdzanie błędów)
                globals.MPCAckNum++;
                globals.unlock();
            } break;

            case REL_MPC: {
                rel_mpc_react(pakiet);
            } break;

            case REQ_SERVICE: {
                sendPacket(&pakiet, pakiet.src, ACK_SERVICE);
            } break;
            
            case ACK_SERVICE: {
                globals.lock();
                globals.ServiceReqNum[pakiet.src] -= 1;
                globals.unlock();
            } break;
            }
        } break;

        case proc_state::INSECTION_MPC: {
            switch ( status.MPI_TAG ) {
            case REQ_MPC: {
                rel_req_mpc(pakiet);
            } break;

            case ACK_MPC: {
                println("ERROR: GOT ACK_MPC in state INSECTION_MPC. SOMETHING WENT WRONGG!!!!");
            } break;

            case REL_MPC: {
                rel_mpc_react(pakiet);
            } break;

            case REQ_SERVICE: {
                sendPacket(&pakiet, pakiet.src, ACK_SERVICE);
            } break;
            
            case ACK_SERVICE: {
                globals.lock();
                globals.ServiceReqNum[pakiet.src] -= 1;
                globals.unlock();
            } break;
            }
        } break;

        case proc_state::WAIT_SERVICE: {
            switch ( status.MPI_TAG ) {
            case REQ_MPC: {
                rel_req_mpc(pakiet);
            } break;

            case ACK_MPC: {
                println("ERROR: GOT ACK_MPC in state INSECTION_MPC. SOMETHING WENT WRONGG!!!!");
            } break;

            case REL_MPC: {
                rel_mpc_react(pakiet);
            } break;

            case REQ_SERVICE: {
                auto sender = pakiet.src;
                auto ts = pakiet.ts;
                globals.lock();
                if (ts < globals.serviceReqTs or (pakiet.ts == globals.serviceReqTs and sender < rank )){
                    sendPacket(&pakiet, pakiet.src, ACK_SERVICE);
                } else {
                    globals.ServiceWaitQueue.push_back(sender);
                }

                globals.unlock();

            } break;
            
            case ACK_SERVICE: {
                globals.lock();
                globals.ServiceReqNum[pakiet.src] -= 1;
                globals.unlock();
            } break;
            }
        } break;

        case proc_state::INSECTION_SERVICE: {
            switch ( status.MPI_TAG ) {
            case REQ_MPC: {
                rel_req_mpc(pakiet);
            } break;

            case ACK_MPC: {
                println("ERROR: GOT ACK_MPC in state INSECTION_MPC. SOMETHING WENT WRONGG!!!!");
            } break;

            case REL_MPC: {
                rel_mpc_react(pakiet);
            } break;

            case REQ_SERVICE: {
                globals.lock();                
                globals.ServiceWaitQueue.push_back(pakiet.ts);
                globals.unlock();                
            } break;
            
            case ACK_SERVICE: {
                globals.lock();
                globals.ServiceReqNum[pakiet.src] -= 1;
                globals.unlock();
            } break;
            }
        } break;


        default:
            debug("TODO dodaj stan bo nie wiem jak odpowiedzieć");
        }
    }
}
