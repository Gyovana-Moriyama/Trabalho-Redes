all:
	g++ client.cpp -o client -lstdc++
	g++ server.cpp -o server -lstdc++

run_client:
	./client

run_server:
	./server
