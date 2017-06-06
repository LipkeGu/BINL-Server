CXX = g++
CXXFLAGS = -pthread -pedantic -Wall
all: binlsvc

binlsvc: clean main.o Client.o Connection.o EventLog.o Packet.o FileSystem.o Functions.o Server.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) Main.o Client.o Connection.o EventLog.o Packet.o Functions.o FileSystem.o Server.o -o binlsvc

main.o: BINL-Server/src/Main.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Main.cpp

Client.o: BINL-Server/src/Client.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Client.cpp

Connection.o: BINL-Server/src/Connection.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Connection.cpp

EventLog.o: BINL-Server/src/EventLog.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/EventLog.cpp

Packet.o: BINL-Server/src/EventLog.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Packet.cpp

Server.o: BINL-Server/src/Server.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/Server.cpp

Functions.o: BINL-Server/src/Functions.cpp
	$(CXX) $(CXXFLAGS) -c sBINL-Server/rc/Functions.cpp

FileSystem.o: BINL-Server/src/FileSystem.cpp
	$(CXX) $(CXXFLAGS) -c BINL-Server/src/FileSystem.cpp

clean:
	rm -rf *.o binlsvc
