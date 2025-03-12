.PHONY: all

all: test

.PHONY: test
test:
	clang++ -std=c++23 -o test test.cc leptjson.cc -I . -v -g
	./test