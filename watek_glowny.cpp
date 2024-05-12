#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    while (true) {
	switch (stan) {
	    case proc_state::REST:
		{
			debug("Stan: REST");
			if (random() % 100 < 25) {
				debug("Czas przegonić czytlników UHUHU. Zmieniam stan na chęć wejścia do sekcji krytycznej");

				int min_data_v = 10000;
				int min_data_ind = 0;

				globals.lock();
				for (int i = 0; i < globals.MPCWaitQueueArray.size(); i++){
					if (globals.MPCWaitQueueArray[i].size() < min_data_v){
						min_data_ind = i;
						min_data_v = globals.MPCWaitQueueArray[i].size();
					} 
				}

				globals.MPCIdx = min_data_ind;
				
				pthread_mutex_lock( &lamport_clock_mutex );

				// Dodać swój REQ(ts) do kolejki MPCWaitQueueArray[]MPCIdx].
				globals.MPCWaitQueueArray[ globals.MPCIdx ].emplace( lamport_clock, rank );
				auto* pkt = new packet_t();

				pkt->mpc_id = globals.MPCIdx;

				broadcastPacket(pkt, mess_t::REQ_MPC, lamport_clock);
				globals.MPCAckNum = 0;
				lamport_clock++;

				pthread_mutex_unlock( &lamport_clock_mutex );
				
				changeState(proc_state::WAIT_MPC);
				
				globals.unlock();
			}
		} break;
		
	}


        sleep(SEC_IN_STATE);
    }
}
