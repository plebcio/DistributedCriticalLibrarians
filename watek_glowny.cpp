#include "main.h"
#include "util.h"
#include "watek_glowny.h"
#include <memory>
#include <algorithm>

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;

    while (true) {
	switch (stan) {
	    case proc_state::REST: {
			if (random() % 100 >= 25) {
				debug("REST: Sluchajcie, ja sie uwielbiam \"\"\"lenic\"\"\"...");
				break;
			}

			// losujemy ile chcemy przgornic
			int czyt = (random() % MAX_CZYT) + MIN_CZYT;
			debug("REST: Czas przegonic %d czytelnikow UHUHU. Chce MPC %d", czyt, globals.MPCIdx);

			int min_data_v = 10000;
			int min_data_ind = 0;


			globals.lock();

			globals.ReadersRand = czyt;

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
			auto pkt = std::make_unique<packet_t>();

			pkt->mpc_id = globals.MPCIdx;

			broadcastPacket(pkt.get(), REQ_MPC, lamport_clock);
			globals.MPCAckNum = 0;
			lamport_clock++;

			pthread_mutex_unlock( &lamport_clock_mutex );
			
			changeState(proc_state::WAIT_MPC);
			
			globals.unlock();
		} break;

		case proc_state::WAIT_MPC: {
			debug("WAIT_MPC: Ale kolejka do tego MPC (%d)", globals.MPCIdx);
			bool can_enter = true;
			globals.lock();
			if (globals.MPCAckNum != size - 1){
				can_enter = false;
			} else {
				auto it = globals.MPCWaitQueueArray[ globals.MPCIdx ].begin();
				// this should never happen
				if (it == globals.MPCWaitQueueArray[ globals.MPCIdx ].end()){
					println("ERROR: globals.MPCWaitQueueArray[ globals.MPCIdx ] is empty when this proc is in queue. Big error");
					exit(0x45);
				}
				if (it->proc_id != rank){
					can_enter = false;
				}
			}
			globals.unlock();
			
			if (!can_enter){
				// this should break out of switch case (?)
				break;
			}

			debug("WAIT_MPC: W koncu popedze jakichs czytelnikow (%d)", globals.ReadersRand);
			changeState(proc_state::INSECTION_MPC);
		} break;

        case proc_state::INSECTION_MPC: {
			if (random() % 100 >= 20) {
				debug("INSECTION_MPC (%d): Poganianiu nie widac konca", globals.MPCIdx);
				break;
			}
            // czas wychodzic
			globals.lock();
			
			if (globals.MPCStateArray[globals.MPCIdx] > globals.ReadersRand ){
				debug("INSECTION_MPC (%d): Pogonilem wszystkich, czas na relaks", globals.MPCIdx);
				globals.MPCStateArray[globals.MPCIdx] -= globals.ReadersRand;
				
				auto* pkt = new packet_t();
				pkt->mpc_id = globals.MPCIdx;
				pkt->mpi_state = globals.MPCStateArray[globals.MPCIdx];

				pthread_mutex_lock( &lamport_clock_mutex );
				lamport_clock++;
				broadcastPacket(pkt, REL_MPC, lamport_clock);				
				pthread_mutex_unlock( &lamport_clock_mutex );
				delete pkt;

				changeState(proc_state::REST);
			} else {
				debug("INSECTION_MPC (%d): MPC? Bardziej NPC, sie zepsulo ustrojstwo.", globals.MPCIdx);
				globals.ReadersRand -= globals.MPCStateArray[globals.MPCIdx];
				globals.MPCStateArray[globals.MPCIdx] = 0;

				auto* pkt = new packet_t();
				pthread_mutex_lock( &lamport_clock_mutex );
				lamport_clock++;
				broadcastPacket(pkt, REQ_SERVICE, lamport_clock);				
				
				globals.serviceReqTs = lamport_clock; 
				pthread_mutex_unlock( &lamport_clock_mutex );

				delete pkt;

				for (int i = 0; i < globals.ServiceReqNum.size(); i++){
					globals.ServiceReqNum[i]++;
				}

				changeState(proc_state::WAIT_SERVICE);
			}

            globals.unlock();

        } break;

		case proc_state::WAIT_SERVICE: {
			debug("WAIT_SERVICE: Najpierw kolejka do MPC, a teraz do serwisu...");
			bool can_enter = false;
			globals.lock();

			auto num_zero = std::count_if(globals.ServiceReqNum.begin(), globals.ServiceReqNum.end(), [](auto e) {return e == 0;});
			debug("WAIT_SERVICE: Zebrałem %d zgod...", num_zero);
			
			if (num_zero >= size - NUM_SERVICE){
				can_enter = true;
			}

			globals.unlock();
			
			if (!can_enter){
				// this should break out of switch case (?)
				break;
			}

			debug("W koncu doczekalem sie serwisanta.");
			changeState(proc_state::INSECTION_SERVICE);
		} break;

		case proc_state::INSECTION_SERVICE: {
			if (random() % 100 >= 30) {
				debug("INSECTION_SERVICE: Naprawiamy MPC %d", globals.MPCIdx);
				break;
			}
			debug("INSECTION_SERVICE: MPC %d juz jest sprawny", globals.MPCIdx);

            // czas wychodzic
			globals.lock();
			
			globals.MPCStateArray[globals.MPCIdx] = BASE_MPC_STATE;
			
			auto* pkt = new packet_t();

			//  this info is sent out when we exit MPC section
			// pkt->mpc_id = globals.MPCIdx;
			// pkt->mpi_state = globals.MPCStateArray[globals.MPCIdx];

			pthread_mutex_lock( &lamport_clock_mutex );
			lamport_clock++;
			for (int dest: globals.ServiceWaitQueue){
				sendPacketNoIncOnTs(pkt, dest, ACK_SERVICE);
			}
			pthread_mutex_unlock( &lamport_clock_mutex );

			delete pkt;

			globals.ServiceWaitQueue.clear();
            globals.unlock();

			debug("INSECTION_SERVICE: Naprawione - czas na dalsze poganianie");
			changeState(proc_state::INSECTION_MPC);

		} break;

		default:
			debug("TODO dodaj stan bo nie wiem co robic :(");
	}

	sleep(SEC_IN_STATE);

    }
}
