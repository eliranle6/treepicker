# Builds everything into build/: the library of the picker's core, the programs using it, and the
# tests. `make help` lists the targets.

CXX ?= g++
CXXFLAGS ?= -O2 -g
CXXFLAGS += -std=c++17 -Wall -Werror
CPPFLAGS += -Isrc -MMD -MP
LDLIBS = -lncursesw
TEST_LDLIBS = -lgtest_main -lgtest -pthread
PREFIX ?= /usr/local

BUILD = build

# The core, shared by all programs
LIB_SOURCES = $(filter-out %/main.cpp src/dirpicker/% src/dirdiff/%, $(wildcard src/*/*.cpp))
LIB = $(BUILD)/libcpptree.a

# Each program: build/<name> from its dir's sources
PROGRAMS = $(BUILD)/dirpicker $(BUILD)/dirdiff $(BUILD)/picker
DIRPICKER_SOURCES = $(wildcard src/dirpicker/*.cpp)
PICKER_SOURCES = src/picker/main.cpp

# Each test: build/tests/<name> from src/*/tests/<name>.cpp
TEST_SOURCES = $(wildcard src/*/tests/test_*.cpp)
TESTS = $(patsubst %.cpp,$(BUILD)/tests/%,$(notdir $(TEST_SOURCES)))

objects = $(patsubst src/%.cpp,$(BUILD)/obj/%.o,$(1))

.PHONY: all test install clean help
all: $(PROGRAMS)

help:
	@echo "make            build the programs into $(BUILD)/"
	@echo "make test       build and run the tests"
	@echo "make install    copy the programs to \$$PREFIX/bin (PREFIX=$(PREFIX))"
	@echo "make clean      remove $(BUILD)/"

$(BUILD)/obj/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(LIB): $(call objects,$(LIB_SOURCES))
	$(AR) rcs $@ $^

$(BUILD)/dirpicker: $(call objects,$(DIRPICKER_SOURCES)) $(LIB)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/dirdiff: $(call objects,src/dirdiff/main.cpp) $(LIB)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD)/picker: $(call objects,$(PICKER_SOURCES)) $(LIB)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Each test's binary depends on its own source's object (written out per test, since its source
# may be in any src/*/tests dir)
$(foreach source,$(TEST_SOURCES),\
	$(eval $(BUILD)/tests/$(basename $(notdir $(source))): $(call objects,$(source)) $(LIB)))
$(TESTS):
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) $(TEST_LDLIBS) -o $@

# Runs every test, even after a failure, and fails if any did. A binary that ran no tests fails
# too, since gtest passes those (e.g. when the tests weren't linked in).
test: all $(TESTS)
	@failed=""; \
	for t in $(TESTS); do \
		if $$t > $$t.log 2>&1 && grep -q '^\[  PASSED  \] [1-9]' $$t.log; then \
			echo "PASS: $$(basename $$t) ($$(grep -o '^\[  PASSED  \] [0-9]*' $$t.log | grep -o '[0-9]*$$') tests)"; \
		else echo "FAIL: $$(basename $$t) (see $$t.log)"; failed=1; fi; \
	done; \
	test -z "$$failed"

install: $(PROGRAMS)
	install -d $(DESTDIR)$(PREFIX)/bin
	install $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin

clean:
	rm -rf $(BUILD)

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
