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

.PHONY: test-null
test-null:
	clang++ -std=c++23 -o test_null lept_test_null.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-null
run-test-null: test-null
	./test_null

.PHONY: clean
clean:
	rm -f test
