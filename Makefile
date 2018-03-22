default: client server client1 server1

client:
	g++ quic.cpp  test_client.cpp -o client -lssl -lcrypto -lpthread -w  -std=c++11

server:
	g++ quic.cpp  test_server.cpp -o server -lssl -lcrypto -lpthread -w  -std=c++11

client1:
	g++ quic.cpp  test_client1.cpp -o client1 -lssl -lcrypto -lpthread -w  -std=c++11

server1:
	g++ quic.cpp  test_server1.cpp -o server1 -lssl -lcrypto -lpthread -w  -std=c++11


clean:
	rm client
	rm server
	rm server1
	rm client1
