.PHONY: all

all: test

format:

.PHONY: test
test:
	clang++ -std=c++23 -o test test.cc leptjson.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test
run-test:
	./test

.PHONY: clean
clean:
	rm -f test
