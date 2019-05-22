Gdyby po uruchomieniu zmieniły się kolorki terminala, wpisujemy polecenie
"reset" do przywrócenia standardowych ustawień terminala.

Kompilacja:

1. make clear; make (make clear usuwa poprzednie pliki wykonywalne i obiektowe)

Wprowadzanie zegarów Lamporta:

1. Dodać zmienną globalną reprezentującą zegar skalarny Lamporta

2. Zmodyfikować makra println (w main.h), by wyświetlały zegar Lamporta 

3. Zwiększanie zegara przy sendPacket (w init.c) oraz max(bieżący, pakiet)+1 przy odbieraniu (w wątku komunikacyjnym comFunc w main.c)

4. MPI_Send oraz MPI_Recv są wywoływane przez różne wątki, więc dostępy do zmiennej
    reprezentującej zegar lamporta należy chronić muteksem (np. nowym albo już istniejącym)

Modyfikowanie typu pakietu:

1. w main.h szukamy struct packet_t i dodajemy pole np. int nowe_pole; zwiększamy FIELDNO

2. w init.c szukamy funkcji inicjuj i tam zamieniamy dodajemy typ pola do MPI_Datatype typy[3] = {MPI_INT, MPI_INT, MPI_INT), wyliczamy offset dla nowego pola

3. make clear; make
