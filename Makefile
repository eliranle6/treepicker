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
LIB_SOURCES = $(filter-out %/main.cpp src/dirpicker/%, $(wildcard src/*/*.cpp))
LIB = $(BUILD)/libcpptree.a

# Each program: build/<name> from its dir's sources
PROGRAMS = $(BUILD)/dirpicker $(BUILD)/picker
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

$(BUILD)/picker: $(call objects,$(PICKER_SOURCES)) $(LIB)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

# A test's source is found by its name, in whichever src/*/tests dir it is
.SECONDEXPANSION:
$(TESTS): $(BUILD)/tests/%: $$(call objects,$$(filter %/$$*.cpp,$(TEST_SOURCES))) $(LIB)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) $(TEST_LDLIBS) -o $@

# Runs every test, even after a failure, and fails if any did
test: $(TESTS)
	@failed=""; \
	for t in $(TESTS); do \
		if $$t > $$t.log 2>&1; then echo "PASS: $$(basename $$t)"; \
		else echo "FAIL: $$(basename $$t) (see $$t.log)"; failed=1; fi; \
	done; \
	test -z "$$failed"

install: $(PROGRAMS)
	install -d $(DESTDIR)$(PREFIX)/bin
	install $(PROGRAMS) $(DESTDIR)$(PREFIX)/bin

clean:
	rm -rf $(BUILD)

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
