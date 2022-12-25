BUILD_DIR = build

CHESS_SRC = src/chess.c src/list.c src/move.c src/board.c src/helpers.c src/perft.c src/prettyprint.c src/eval.c src/minmax.c src/zobrist.c src/book.c
CHESS_OBJ = $(addprefix $(BUILD_DIR)/, $(CHESS_SRC:%.c=%.o))

TEST_DIR = tests
TEST_SRC = $(wildcard $(TEST_DIR)/test_*.c)
TEST_OBJ = $(addprefix $(BUILD_DIR)/, $(TEST_SRC:%.c=%))
TEST_TARGET = $(notdir $(TEST_OBJ))
DEBUG_TARGET = $(addprefix gdb_, $(TEST_TARGET))
VALGRIND_TARGET = $(addprefix valgrind_, $(TEST_TARGET))

CC = gcc
CC_FLAGS = -Wall -Wextra -g -std=c11 -O3

.PHONY: all
all: build build_tests     # Build everything but runs nothing

.PHONY: run
run: all_tests             # Run all tests (alias)

.PHONY: build
build: $(CHESS_OBJ)     # Build the chess program

.PHONY: build_tests
build_tests: $(TEST_OBJ)   # Build the tests

.PHONY: all_tests
all_tests: $(TEST_TARGET)  # Run all tests

.PHONY: list_tests
list_tests:
	@echo "List of all test targets"
	@echo "------------------------"
	@for test in $(TEST_TARGET) ; do \
		echo "make $$test"; \
	done

BASH_COLOR_RED = \033[0;31m
BASH_COLOR_NONE = \033[0m
BASH_COLOR_GREEN = \033[32m

.PHONY: $(TEST_TARGET)   # Run a specific test
$(TEST_TARGET): %: $(BUILD_DIR)/tests/%
	@echo ">>> Running $<"
	@./$< && echo -e "<<< $(BASH_COLOR_GREEN)OK$(BASH_COLOR_NONE)" \
		|| echo -e "<<< $(BASH_COLOR_RED)FAILED$(BASH_COLOR_NONE)"

$(CHESS_OBJ): $(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ -c $<


$(BUILD_DIR)/tests/test_%: tests/test_%.c $(BUILD_DIR)/src/chess.o $(BUILD_DIR)/src/list.o $(BUILD_DIR)/src/move.o $(BUILD_DIR)/src/board.o $(BUILD_DIR)/src/helpers.o $(BUILD_DIR)/src/perft.o $(BUILD_DIR)/src/prettyprint.o $(BUILD_DIR)/src/eval.o $(BUILD_DIR)/src/minmax.o $(BUILD_DIR)/src/zobrist.o $(BUILD_DIR)/src/book.o
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o$@ $^


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

