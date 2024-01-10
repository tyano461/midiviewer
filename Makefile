CC 		:= g++
CFLAGS 	:= -g -O0
LDFLAGS := 
INCLUDE :=
TARGET	:= midiviewer

CFLAGS += $(INCLUDE)

OBJS := main.o midi.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

.cpp.o:
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(TARGET) $(OBJS)
