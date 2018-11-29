all: main

main: test.cpp 
	g++ test.cpp -o main -lpthread -mcmodel=large

.PHONY: clean

clean:
	rm -f main
