PROGS = client rwlServer attacker
CC = gcc
CFLAGS = -std=gnu99 -lm -lpthread -o

all: $(PROGS)

rwlServer: rwlServer.c
	$(CC) $(CFLAGS) rwlServer rwlServer.c

client: client.c
	$(CC) $(CFLAGS) client client.c

attacker: attacker.c
	$(CC) $(CFLAGS) attacker attacker.c

clean:
	rm $(PROGS)