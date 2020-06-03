all:
	gcc -g -pthread ./src/client.cpp -o client -lstdc++
	gcc -g -pthread ./src/server.cpp -o server -lstdc++

run_client:
	./client

run_server:
	./server

clean:
	rm client
	rm server