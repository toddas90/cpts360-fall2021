CC := gcc
debug: CFLAGS :=  -g -Wall
release: CFLAGS := -O2 --pipe -march=native

TARGET := final

INCLUDES := -I./include
SRCS := ./src/*

OBJS := $(SRCS:.c=.o)

.PHONY: clean

debug : $(TARGET)
	./newdisk
	@echo Debug build finished

release : $(TARGET)
	./newdisk
	@echo Release build finished

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c  $< -o $@

clean:
	$(RM) *.o *~ $(TARGET)
