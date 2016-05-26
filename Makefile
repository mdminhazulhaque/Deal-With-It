CFLAGS = `pkg-config --cflags opencv`
LINKER = `pkg-config --libs opencv`
BINARY = dealwithit
all: DealWithIt.o
	g++ -o $(BINARY) main.cpp DealWithIt.o $(LINKER)
DealWithIt.o: DealWithIt.cpp
	g++ -g -c -Wall DealWithIt.cpp $(CFLAGS)
clean:
	rm -f *~ *.o $(BINARY)
