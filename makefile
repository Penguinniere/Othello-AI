main:
	g++ main.cpp board.cpp monte_carlo.cpp -O2 -std=c++11 -o r04922030
windows:
	g++ main.cpp board.cpp monte_carlo.cpp -O2 -std=c++11 -lws2_32 -o r04922030
clean:
	rm r04922030
