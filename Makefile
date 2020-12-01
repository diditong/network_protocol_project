.PHONY: all package serverA serverB mainserver

%.o: %.cpp
	g++ -ggdb3 -std=c++17 -Wall -o $@ -c $^

all: client.o common.o serverA.o serverB.o servermain.o
	g++ -ggdb3 -std=c++17 -o client client.o common.o
	g++ -ggdb3 -std=c++17 -o serverA serverA.o common.o
	g++ -ggdb3 -std=c++17 -o serverB serverB.o common.o
	g++ -ggdb3 -std=c++17 -pthread -o servermain servermain.o common.o

clean:
	rm -f *.o client serverA serverB servermain ee450_jiashuot_session3.tar.gz

serverA:
	./serverA

serverB:
	./serverB

mainserver:
	./servermain

package:
	tar cvf ee450_jiashuot_session3.tar *.cpp *.h Makefile README
	gzip ee450_jiashuot_session3.tar
