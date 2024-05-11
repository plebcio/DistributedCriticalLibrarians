SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.cpp=.h)
FLAGS=-DDEBUG -g -fpermissive
# FLAGS=-g 

all: main

main: $(SOURCES) $(HEADERS) Makefile
	mpicxx $(SOURCES) $(FLAGS) -o main

debug: $(SOURCES) $(HEADERS) Makefile
	mpicxx $(SOURCES) $(FLAGS) -DDEBUG -o main

clear: clean

clean:
	rm main 
	rm a.out

run: main Makefile
	mpirun -oversubscribe -np 8 ./main

zip:
	zip -r proj.zip ./