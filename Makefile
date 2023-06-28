BUILD_DIR = build

EXERCISE_SRC = src/board.c src/helpers.c src/prettyprint.c src/movegen.c src/move.c src/magic.c src/perft.c src/eval.c src/minmax.c src/searchdata.c src/zobrist.c src/pq.c src/database.c src/parse.c src/san.c src/features.c
EXERCISE_OBJ = $(addprefix $(BUILD_DIR)/, $(EXERCISE_SRC:%.c=%.o))

GUI_SRC = src/gui.c
TRAIN_SRC = src/train.c

TEST_DIR = tests
TEST_SRC = $(wildcard $(TEST_DIR)/test_*.c)
TEST_OBJ = $(addprefix $(BUILD_DIR)/, $(TEST_SRC:%.c=%))
TEST_TARGET = $(notdir $(TEST_OBJ))
DEBUG_TARGET = $(addprefix gdb_, $(TEST_TARGET))
VALGRIND_TARGET = $(addprefix valgrind_, $(TEST_TARGET))

CC = gcc
CC_FLAGS = -Wall -Wextra -O3 -g

.PHONY: all
all: build build_tests $(BUILD_DIR)/gui/gui $(BUILD_DIR)/train/train  # Build everything but runs nothing


.PHONY: gui
gui: build $(BUILD_DIR)/gui/gui 

.PHONY: train
train: build $(BUILD_DIR)/train/train 

.PHONY: run
run: all_tests             # Run all tests (alias)

.PHONY: build
build: $(EXERCISE_OBJ)     # Build the exercise

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

$(EXERCISE_OBJ): $(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ -c $<


$(BUILD_DIR)/tests/test_%: tests/test_%.c $(EXERCISE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o$@ $^ -L. -llinalg

$(BUILD_DIR)/gui/gui: $(GUI_SRC) $(EXERCISE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o$@ $^ -L. -llinalg

$(BUILD_DIR)/train/train: $(TRAIN_SRC) $(EXERCISE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o$@ $^ -L. -llinalg

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

