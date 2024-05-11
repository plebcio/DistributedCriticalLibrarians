SOURCES=$(wildcard *.c)
HEADERS=$(SOURCES:.c=.h)
# FLAGS=-DDEBUG -g
FLAGS=-g

all: main tags

main: $(SOURCES) $(HEADERS) Makefile
	mpicc $(SOURCES) $(FLAGS) -o main

debug: $(SOURCES) $(HEADERS) Makefile
	mpicc $(SOURCES) $(FLAGS) -DDEBUG -o main

clear: clean

clean:
	rm main 
	rm a.out

tags: ${SOURCES} ${HEADERS}
	ctags -R .

run: main Makefile tags
	mpirun -oversubscribe -np 8 ./main

zip:
	zip -r proj.zip ./