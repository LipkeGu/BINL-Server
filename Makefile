CXX = g++
CXXFLAGS = -pthread -pedantic -Wall
all: binlsvc

binlsvc: clean main.o Client.o Connection.o Packet.o Functions.o Server.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) Main.o Client.o Connection.o Packet.o Functions.o Server.o -o binlsvc

main.o: BINL-Server/src/Main.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Main.cpp

Client.o: BINL-Server/src/Client.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Client.cpp

Connection.o: BINL-Server/src/Connection.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Connection.cpp

Packet.o: BINL-Server/src/Packet.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Packet.cpp

Functions.o: BINL-Server/src/Functions.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Functions.cpp

Server.o: BINL-Server/src/Server.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Server.cpp

clean:
	rm -rf *.o binlsvc
