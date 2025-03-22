.PHONY: all

all: test

format:
	clang-format -i lept_json.cc lept_json.h test.cc

.PHONY: test
test:
	clang++ -std=c++23 -o test test.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test
run-test: test
	./test

.PHONY: clean
clean:
	rm -f test
