BUILD_DIR = build
BIN_DIR = bin
LIB_DIR = lib
TEST_DIR = tests
TMP_DIR = tmp

ENGINE_CORE_SRC = $(wildcard src/engine-core/*.c)
ENGINE_CORE_OBJ = $(addprefix $(BUILD_DIR)/, $(ENGINE_CORE_SRC:src/%.c=%.o))

PARSING_SRC = $(wildcard src/parsing/*.c)
PARSING_OBJ = $(addprefix $(BUILD_DIR)/, $(PARSING_SRC:src/%.c=%.o))

ORDERING_SRC = $(wildcard src/ordering/*.c)
ORDERING_OBJ = $(addprefix $(BUILD_DIR)/, $(ORDERING_SRC:src/%.c=%.o))

EVAL_SRC = $(wildcard src/train-eval/*.c)
EVAL_OBJ = $(addprefix $(BUILD_DIR)/, $(EVAL_SRC:src/%.c=%.o))

UCI_ENGINE_SRC = src/uci.c
TRAIN_ORDERING_SRC = src/train_ordering.c
TEST_ORDERING_SRC = src/test_ordering.c
TRAIN_EVAL_SRC = src/train.c

TEST_SRC = $(wildcard $(TEST_DIR)/test_*.c)
TEST_BIN = $(addprefix $(BUILD_DIR)/, $(TEST_SRC:%.c=%))
TEST_TARGET = $(notdir $(TEST_BIN))

CC = clang
CC_FLAGS = -Wall -Wcast-qual -Wextra -Wshadow -Wmissing-declarations -O3 -I.

.PHONY: all
all: uci_engine train_ordering test_ordering train_eval library

.PHONY: engine_core
engine_core: $(ENGINE_CORE_OBJ)

.PHONY: parser
parser: $(PARSING_OBJ)

.PHONY: ordering
ordering: $(ORDERING_OBJ)

.PHONY: eval
eval: $(EVAL_OBJ)

.PHONY: uci_engine
uci_engine: engine_core parser ordering $(BIN_DIR)/uci_engine

.PHONY: train_ordering
train_ordering: engine_core parser ordering $(BIN_DIR)/train_ordering

.PHONY: test_ordering
test_ordering: engine_core parser ordering $(BIN_DIR)/test_ordering

.PHONY: train_eval
train_eval: engine_core parser ordering eval $(BIN_DIR)/train_eval

.PHONY: library
library: engine_core parser ordering $(LIB_DIR)/libchess.so

.PHONY: build_tests
build_tests: $(TEST_BIN)   # Build the tests

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

$(ENGINE_CORE_OBJ): $(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	# $(CC) $(CC_FLAGS) -fno-exceptions -flto=full -o $@ -c $<
	$(CC) $(CC_FLAGS) -fno-exceptions -o $@ -c $<

$(PARSING_OBJ): $(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	# $(CC) $(CC_FLAGS) -fno-exceptions -flto=full -o $@ -c $<
	$(CC) $(CC_FLAGS) -fno-exceptions -o $@ -c $<

$(ORDERING_OBJ): $(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ -c $<

$(EVAL_OBJ): $(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ -c $<

$(LIB_DIR)/libchess.so: $(UCI_ENGINE_SRC) $(ENGINE_CORE_OBJ) $(ORDERING_OBJ) $(PARSING_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -shared -fPIC -o $@ $^ -lm

$(BIN_DIR)/uci_engine: $(UCI_ENGINE_SRC) $(ENGINE_CORE_OBJ) $(ORDERING_OBJ) $(PARSING_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ $^ -lm

$(BIN_DIR)/train_ordering: $(TRAIN_ORDERING_SRC) $(PARSING_OBJ) $(ENGINE_CORE_OBJ) $(ORDERING_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ $^ -lm

$(BIN_DIR)/test_ordering: $(TEST_ORDERING_SRC) $(PARSING_OBJ) $(ENGINE_CORE_OBJ) $(ORDERING_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ $^ -lm

$(BIN_DIR)/train_eval: $(TRAIN_EVAL_SRC) $(PARSING_OBJ) $(ENGINE_CORE_OBJ) $(EVAL_OBJ) $(ORDERING_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ $^ -L./lib -llinalg -lm

$(TEST_BIN): $(BUILD_DIR)/%: %.c $(ENGINE_CORE_OBJ) $(ORDERING_OBJ) $(PARSING_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -o $@ $^ -lm


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(TMP_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(LIB_DIR)/libchess.so