CC := gcc
CFLAGS := -g -Wall -pipe -march=native

.PHONY: clean

server : ./src/server.c ./include/commands.h
	$(CC) $(CFLAGS) -o server ./src/server.c ./src/commands.c ./include/commands.h

client : ./src/client.c ./include/commands.h
	$(CC) $(CFLAGS) -o client ./src/client.c ./src/commands.c ./include/commands.h

clean:
	$(RM) *.o *~ $(TARGET)
