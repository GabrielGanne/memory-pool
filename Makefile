TARGET = libmpool.so

include config.mk

# compilation options
DEBUG ?= 0
MEMCHECK ?= 0
ASAN ?= 0
TSAN ?= 0  # requires gcc >= 7 (gcc.gnu.org/bugzilla/show_bug.cgi?id=67308)
PREFIX ?= /usr

# use all available procs by default
NPROC ?= $(shell grep -c "processor" /proc/cpuinfo)
MAKEFLAGS += -j$(NPROC)

# env
CC ?= gcc
makefile_full_path := $(abspath $(lastword $(MAKEFILE_LIST)))
TOPDIR := $(patsubst %/,%,$(dir $(makefile_full_path)))

ifeq ($(CC),clang)
    CFLAGS_WARN = -Weverything
    CFLAGS_WARN += -Wno-padded
else
    CFLAGS_WARN = -Wall -Wextra -pedantic -Wshadow -Wstrict-prototypes -Wmissing-prototypes
    CFLAGS_WARN += -Wno-padded
endif

ifeq ($(DEBUG),1)
    CFLAGS_DEBUG += -O0 -g
else
    CFLAGS_DEBUG += -O3 -DNDEBUG
endif

ifeq ($(ASAN).$(TSAN), 1.1)
$(error "cannot enable both address and thread sanitizers")
endif
ifeq ($(ASAN).$(MEMCHECK), 1.1)
$(error "should not enable both address sanitizer and memcheck")
endif

ifeq ($(MEMCHECK), 1)
    CFLAGS_MEMCHECK = -DMEMCHECK
endif

ifeq ($(ASAN), 1)
    CFLAGS_ASAN = -fsanitize=address
    LDFLAGS_ASAN = -lasan
endif

ifeq ($(TSAN), 1)
    CFLAGS_TSAN = -fsanitize=thread
    LDFLAGS_TSAN = -ltsan
endif

CFLAGS_ALL := $(CFLAGS_WARN) $(CFLAGS_DEBUG) $(CFLAGS_MEMCHECK) $(CFLAGS_ASAN) $(CFLAGS_TSAN)
LDFLAGS_ALL := $(LDFLAGS_ASAN) $(LDFLAGS_TSAN)

CPPFLAGS := -pipe -std=gnu11 -I$(TOPDIR)/src/ $(CPPFLAGS_CONFIG) $(CPPFLAGS)
CFLAGS := $(CFLAGS_ALL) $(CFLAGS)
LDFLAGS := $(LDFLAGS_ALL) $(LDFLAGS)


# source definitions
HEADERS = \
	src/common.h \
	src/mpool.h

SOURCES = \
	src/mpool.c

OBJECTS = $(SOURCES:.c=.o)
$(OBJECTS): $(HEADERS)

.INTERMEDIATE: $(OBJECTS)
%.o: %.c 
	$(CC) $(CPPFLAGS) -shared -fPIC $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CC) -shared -fPIC $(CFLAGS) $(LDFLAGS) -o $(TARGET) $^

.PHONY: clean
clean: test_clean
	-@rm -vf $(OBJECTS)
	-@rm -vf $(TARGET)

# syntax check using clang static analyzer
.PHONY: syntax
syntax:
	$(foreach file, $(SOURCES), \
			clang -Weverything $(CPPFLAGS) $(CFLAGS) -fsyntax-only $(file);)

.PHONY: fixstyle
fixstyle: devtools/uncrustify.cfg
	uncrustify -c $^ -l C --replace $(SOURCES) $(HEADERS)

.PHONY: install
install:
	@mkdir -p $(PREFIX)/include $(PREFIX)/lib
	@cp -vf $(TARGET) $(PREFIX)/lib
	@cp -vf $(HEADERS) $(PREFIX)/include

.PHONY: uninstall
uninstall:
	-@rm -vf $(PREFIX)/lib/$(TARGET)
	-@$(foreach header, $(notdir $(HEADERS)), rm -vf $(PREFIX)/include/$(header);)

include test/test.mk
.PHONY: test
test: $(ALL_TESTS) $(TEST_MPOOL_OVERLOAD)
	$(foreach test_sample, $(ALL_TESTS), \
			LD_LIBRARY_PATH=$(TOPDIR) $(TOPDIR)/$(test_sample) || exit 1;)

.PHONY: check
check: test syntax

.PHONY: help
help:
	@echo "# Targets:"
	@echo "build                   - build application"
	@echo "clean                   - clean"
	@echo "syntax                  - run static analyzer"
	@echo "test                    - run tests"
	@echo "check                   - run static checks and functionnal tests"
	@echo "install                 - install to $(PREFIX)"
	@echo "fixstyle                - fix coding style"
	@echo
	@echo "# Config:"
	@echo "LOG2_CPU_CACHELINE_SIZE = $(CONFIG_LOG2_CPU_CACHELINE_SIZE)"
	@echo "LOG2_CPU_PAGE_SIZE      = $(CONFIG_LOG2_CPU_PAGE_SIZE)"
	@echo
	@echo "# Environement:"
	@echo "CC                      = $(CC)"
	@echo "PREFIX                  = $(PREFIX)"
	@echo "DEBUG                   = $(DEBUG)"
	@echo "MEMCHECK                = $(MEMCHECK)"
	@echo "ASAN                    = $(ASAN)"
	@echo "TSAN                    = $(TSAN)"

.PHONY: all
all: $(TARGET)

.DEFAULT_GOAL := all
