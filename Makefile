all:	prog1

prog1: treesearch.cpp
	g++ treesearch.cpp -o treesearch
clean:
	rm -rf treesearch
