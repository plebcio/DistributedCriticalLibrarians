ZADANIE:
Zaimplementować zegary lamporta oraz dowolny algorytm dostępu do sekcji krytycznej.
Zaimplementować wyświetlanie zegarów w makrach println oraz debug.
0) stworzyć zmienną globalną w jakimś pliku .c oraz zapowiedź (extern) w jakimś pliku .h
2) zmodyfikować makra println i debug w pliku main.h by wyświetlały zegar lamporta
3) przy wysyłaniu (sendPacket) pdbijać pole ts i zwiększać lokalny zegary lamporta
4) przy odbieraniu (wątek komunikacyjny) max( ts, lokalny zegar ) +1
5) pamiętać o obwarowaniu dostępu do zegara lamporta muteksami

CO JEST:
w struct packet_t jest już pole "ts" - to będzie timestamp.
Trzeba dodać zmienną clock i obwarować ją muteksami.
