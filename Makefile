.PHONY: all

all: test-all

format:
	clang-format -i lept_json.cc lept_json.h test.cc lept_test_*.cc

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

.PHONY: test-true
test-true:
	clang++ -std=c++23 -o test_true lept_test_true.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-true
run-test-true: test-true
	./test_true

.PHONY: test-false
test-false:
	clang++ -std=c++23 -o test_false lept_test_false.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-false
run-test-false: test-false
	./test_false

.PHONY: test-number
test-number:
	clang++ -std=c++23 -o test_number lept_test_number.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-number
run-test-number: test-number
	./test_number

.PHONY: test-string
test-string:
	clang++ -std=c++23 -o test_string lept_test_string.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-string
run-test-string: test-string
	./test_string

.PHONY: test-array
test-array:
	clang++ -std=c++23 -o test_array lept_test_array.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-array
run-test-array: test-array
	./test_array

.PHONY: test-object
test-object:
	clang++ -std=c++23 -o test_object lept_test_object.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-object
run-test-object: test-object
	./test_object

.PHONY: test-error
test-error:
	clang++ -std=c++23 -o test_error lept_test_error.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-error
run-test-error: test-error
	./test_error

.PHONY: test-stringify
test-stringify:
	clang++ -std=c++23 -o test_stringify lept_test_stringify.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-stringify
run-test-stringify: test-stringify
	./test_stringify

.PHONY: test-equal
test-equal:
	clang++ -std=c++23 -o test_equal lept_test_equal.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-equal
run-test-equal: test-equal
	./test_equal

.PHONY: test-copy-move-swap
test-copy-move-swap:
	clang++ -std=c++23 -o test_copy_move_swap lept_test_copy_move_swap.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-copy-move-swap
run-test-copy-move-swap: test-copy-move-swap
	./test_copy_move_swap

.PHONY: test-access
test-access:
	clang++ -std=c++23 -o test_access lept_test_access.cc lept_json.cc -I . -g -Wall -Wextra -pedantic

.PHONY: run-test-access
run-test-access: test-access
	./test_access

# Target to build all tests
.PHONY: test-all
test-all: test-null test-true test-false test-number test-string test-array test-object test-error test-stringify test-equal test-copy-move-swap test-access

# Target to run all tests
.PHONY: run-test-all
run-test-all: run-test-null run-test-true run-test-false run-test-number run-test-string run-test-array run-test-object run-test-error run-test-stringify run-test-equal run-test-copy-move-swap run-test-access

.PHONY: clean
clean:
	rm -rf test test_null* test_true* test_false* test_number* test_string* test_array* test_object* test_error* test_stringify* test_equal* test_copy_move_swap* test_access*
