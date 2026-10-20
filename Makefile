# pegasus
#
#   make build              configure + build everything
#   make test               build, then run the full test suite (ctest)
#   make test-all           build, then run every test file, then the combined one
#   make test-file FILE=test_value          run one test file
#   make test-file FILE=test_chunk ARGS="-s"  with Catch2 args
#   make run                build, then run the pegasus binary
#   make list-tests         list Catch2 test cases per test file
#   make clean              remove build artifacts (keep config)
#   make distclean          remove the whole build directory

BUILD_DIR  ?= build
BUILD_TYPE ?= Release
JOBS       ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Accepts "test_value", "test_value.cpp" or "tests/common/test_value.cpp"
TEST_NAME := $(basename $(notdir $(FILE)))
ARGS      ?=

# Test sources live in subdirectories (tests/common, tests/execution, ...).
# Prune any stray build tree (e.g. tests/build) so it is never picked up.
TEST_SOURCES := $(shell find tests \( -name build -o -name $(notdir $(BUILD_DIR)) \) -prune -o -name 'test_*.cpp' -print | sort)
TEST_SRC     := $(firstword $(filter %/$(TEST_NAME).cpp $(TEST_NAME).cpp,$(TEST_SOURCES)))

CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

.PHONY: all build configure test test-all test-file list-tests run clean distclean help

all: build

## Configure and build all targets (pegasus + tests)
configure:
	cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS)

build: configure
	cmake --build $(BUILD_DIR) -j $(JOBS)

## Build and run the full test suite (via ctest)
test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

## Build and run every test file's executable, then the combined binary
test-all: build
	@set -e; for f in $(TEST_SOURCES); do \
	    name=$$(basename $$f .cpp); \
	    echo "==> $$name"; \
	    $(BUILD_DIR)/tests/$$name --reporter console; \
	done
	@echo "==> test_all (combined)"
	$(BUILD_DIR)/tests/test_all --reporter console

## Run a single test file: make test-file FILE=test_value [ARGS="--filter ..."]
test-file:
	@if [ -z "$(FILE)" ]; then \
	    echo 'usage: make test-file FILE=test_value [ARGS="<catch2 args>"]'; \
	    echo 'known files:'; \
	    for f in $(TEST_SOURCES); do echo "  FILE=$$(basename $$f .cpp)"; done; \
	    exit 1; \
	fi
	@if [ -z "$(TEST_SRC)" ]; then \
	    echo "error: no test source named '$(TEST_NAME).cpp' under tests/"; exit 1; \
	fi
	@if [ ! -f $(BUILD_DIR)/CMakeCache.txt ]; then cmake -S . -B $(BUILD_DIR) $(CMAKE_FLAGS); fi
	cmake --build $(BUILD_DIR) --target $(TEST_NAME) -j $(JOBS)
	$(BUILD_DIR)/tests/$(TEST_NAME) $(ARGS)

## List every Catch2 test case in each test file
list-tests: build
	@set -e; for f in $(TEST_SOURCES); do \
	    name=$$(basename $$f .cpp); \
	    echo "$$name:"; \
	    $(BUILD_DIR)/tests/$$name --list-tests | sed 's/^/  /'; \
	done

## Build and run the main pegasus demo binary
run: build
	$(BUILD_DIR)/src/pegasus

## Delete build outputs but keep the CMake configuration
clean:
	@if [ -d "$(BUILD_DIR)" ]; then cmake --build $(BUILD_DIR) --target clean; fi

## Delete the entire build directory
distclean:
	rm -rf $(BUILD_DIR)

help:
	@grep -E '^## ' Makefile | sed 's/^## //'
