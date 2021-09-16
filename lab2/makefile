CC := gcc
CFLAGS := -O3 -Wall -Werror -pipe -march=native
debug: CFLAGS :=  -g -Wall -Werror -pipe -march=native

TARGET := lab2

INCLUDES := -I./include
SRCS := ./src/*

OBJS := $(SRCS:.c=.o)

.PHONY: clean

release : $(TARGET)
	@echo Release build finished

debug : $(TARGET)
	@echo Debug build finished

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c  $< -o $@

clean:
	$(RM) *.o *~ $(TARGET)
