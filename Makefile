all:
	gcc -pthread client.cpp -o client -lstdc++
	gcc -pthread server.cpp -o server -lstdc++

run_client:
	./client

run_server:
	./server

clean:
	rm client
	rm server