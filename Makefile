all: main

main: test.cpp 
	g++ test.cpp -o main -lpthread -mcmodel=large -std=c++14

.PHONY: clean

clean:
	rm -f main
