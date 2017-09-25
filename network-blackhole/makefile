.PHONY: all
all:	tcpblackhole

.PHONY: clean
clean:
	rm -f tcpblackhole

tcpblackhole: tcpblackhole.cpp
	g++  tcpblackhole.cpp -o tcpblackhole -lpthread -O3

