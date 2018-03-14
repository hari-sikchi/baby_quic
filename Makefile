default: client server

client:
	g++ quic.cpp  test_client.cpp -o client -lssl -lcrypto -lpthread -w  -std=c++11

server:
	g++ quic.cpp  test_server.cpp -o server -lssl -lcrypto -lpthread -w  -std=c++11



clean:
	rm client
	rm server
