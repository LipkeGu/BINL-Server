CXX = g++
CXXFLAGS = -pthread -pedantic -Wall
all: binlsvc

binlsvc: clean main.o Client.o Connection.o EventLog.o Packet.o FileSystem.o Functions.o Server.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) Main.o Client.o Connection.o EventLog.o Packet.o Functions.o FileSystem.o Server.o -o binlsvc

main.o: src/Main.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c src/Main.cpp

Client.o: src/Client.cpp
	$(CXX) $(CXXFLAGS) -c src/Client.cpp

Connection.o: src/Connection.cpp
	$(CXX) $(CXXFLAGS) -c src/Connection.cpp

EventLog.o: src/EventLog.cpp
	$(CXX) $(CXXFLAGS) -c src/EventLog.cpp

Packet.o: src/EventLog.cpp
	$(CXX) $(CXXFLAGS) -c src/Packet.cpp

Server.o: src/Server.cpp
	$(CXX) $(CXXFLAGS) -c src/Server.cpp

Functions.o: src/Functions.cpp
	$(CXX) $(CXXFLAGS) -c src/Functions.cpp

FileSystem.o: src/FileSystem.cpp
	$(CXX) $(CXXFLAGS) -c src/FileSystem.cpp

clean:
	rm -rf *.o binlsvc
