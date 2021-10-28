CC := gcc
debug: CFLAGS :=  -g -O0 -Wall
release: CFLAGS := -O3 -Wall -Werror -pipe -march=native

TARGET := final

INCLUDES := -I./include
SRCS := ./src/*

OBJS := $(SRCS:.c=.o)

.PHONY: clean

debug : $(TARGET)
	@echo Debug build finished

release : $(TARGET)
	@echo Release build finished

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c  $< -o $@

clean:
	$(RM) *.o *~ $(TARGET)

