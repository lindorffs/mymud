CLIENT_EXE = src/client/bin/client.exe
SERVER_EXE = src/server/bin/server.exe

full: client server
full_remake: client_remake server_remake
full_clean: client_clean server_clean

client_clean:
	cd src/client && make clean
client_remake: $(CLIENT_EXE)_remake
$(CLIENT_EXE)_remake: 
	cd src/client && make clean && make -j4
client: $(CLIENT_EXE)
$(CLIENT_EXE): 
	cd src/client && make -j4

server_clean:
	cd src/server && make clean
server_remake: $(SERVER_EXE)_remake
$(SERVER_EXE)_remake:
	cd src/server && make clean && make -j4
server: $(SERVER_EXE)
$(SERVER_EXE):
	cd src/server && make -j4
