CC=g++ -fno-pie -no-pie
CFLAGS=-Iinclude -O2 -Wall -D_GLIBCXX_ISE_CXX11_ABI=1 
SOURCES=src/main.cpp src/place.cpp src/partitioner.cpp
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=place

all: $(SOURCES) bin/$(EXECUTABLE)
	
bin/$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

%.o: %.c ${INCLUDES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o bin/$(EXECUTABLE)
