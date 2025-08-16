CLIENT_EXE = src/client/bin/client.exe
SERVER_EXE = src/server/bin/server.exe

client: $(CLIENT_EXE)

$(CLIENT_EXE): 
	cd src/client && make clean && make -j4
	
server: $(SERVER_EXE)

$(CLIENT_EXE):
	cd src/server && make clean && make -j4