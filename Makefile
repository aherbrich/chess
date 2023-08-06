BUILD_DIR = build

EXERCISE_SRC = src/engine-core/board.c src/engine-core/helpers.c src/engine-core/prettyprint.c src/engine-core/movegen.c src/engine-core/magic.c \
			   src/engine-core/perft.c src/engine-core/eval.c src/engine-core/minmax.c src/engine-core/searchdata.c src/engine-core/zobrist.c src/engine-core/pq.c \
			   src/parsing/parse.c src/parsing/san.c \
			   src/train-eval/database.c src/train-eval/features.c \
			   src/train-ordering/gaussian.c src/train-ordering/factors.c 
EXERCISE_OBJ = $(addprefix $(BUILD_DIR)/, $(EXERCISE_SRC:%.c=%.o))

GUI_SRC = src/gui.c
TRAIN_SRC = src/train.c
TRAIN_ORDERING_SRC = src/ordering.c

TEST_DIR = tests
TEST_SRC = $(wildcard $(TEST_DIR)/test_*.c)
TEST_OBJ = $(addprefix $(BUILD_DIR)/, $(TEST_SRC:%.c=%))
TEST_TARGET = $(notdir $(TEST_OBJ))
DEBUG_TARGET = $(addprefix gdb_, $(TEST_TARGET))
VALGRIND_TARGET = $(addprefix valgrind_, $(TEST_TARGET))


CC = gcc
CC_FLAGS = -Wall -Wcast-qual -Wextra -Wshadow -Wmissing-declarations -O3 #-fno-exceptions -flto=full

.PHONY: all	
all: build build_tests $(BUILD_DIR)/gui/gui $(BUILD_DIR)/train/train $(BUILD_DIR)/train_ordering/train_ordering # Build everything but runs nothing


.PHONY: gui
gui: build $(BUILD_DIR)/gui/gui 

.PHONY: train
train: build $(BUILD_DIR)/train/train 

.PHONY: train_ordering
train_ordering: build $(BUILD_DIR)/train_ordering/train_ordering 

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
	$(CC) $(CC_FLAGS) -o $@ $^ -L./lib -llinalg

$(BUILD_DIR)/gui/gui: $(GUI_SRC) $(EXERCISE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ $^ -L./lib -llinalg

$(BUILD_DIR)/train/train: $(TRAIN_SRC) $(EXERCISE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS)  -o $@  $^ -L./lib -llinalg

$(BUILD_DIR)/train_ordering/train_ordering: $(TRAIN_ORDERING_SRC) $(EXERCISE_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS)  -o $@  $^ -L./lib -llinalg

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

