CC=gcc
CFLAGS=-I.

all:	clean agent manager

agent: src/agent.o
	@$(CC) -o agent src/agent.c src/geolocation.c src/cJSON.c src/payload.c -I. -lpthread -lm

manager: src/manager.o
	@$(CC) -o manager src/manager.o src/cJSON.c src/geolocation.c src/payload.c -I. -lpthread -lm

clean:
	rm -f src/*.o *~ agent manager

