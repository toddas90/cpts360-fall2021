CC := gcc
CFLAGS :=  -g -Wall -Werror
TARGET := lab2

INCLUDES := -I./include
SRCS := ./src/*

OBJS := $(SRCS:.c=.o)

.PHONY: clean

all : $(TARGET)
	@echo Compilation Finished

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c  $< -o $@

clean:
	$(RM) *.o *~ $(TARGET)
