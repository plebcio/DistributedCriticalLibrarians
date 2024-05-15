#ifndef __MY_CLASS_HEADER__
#define __MY_CLASS_HEADER__

#include <type_traits>
#include <vector>
#include <set>

struct request {
    int ts;
    int proc_id;

    request(int ts, int proc_id): ts(ts), proc_id(proc_id) {}

};

// tutaj inline podowuej ze moge mieć ile chce definicji i linker nie krzyczy
// nie jest to fajne rozwiązanie ale cały ten kod to spagetti
inline bool operator<(request const &a, request const &b) {
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

    int serviceReqTs;

    void lock();
    void unlock();
    glob_data(int n_proc);
};


#endif